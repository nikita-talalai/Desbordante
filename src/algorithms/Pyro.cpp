#include <chrono>
#include <mutex>
#include <thread>

#include "FdG1Strategy.h"
#include "KeyG1Strategy.h"
#include "logging/easylogging++.h"
#include "Pyro.h"

std::mutex searchSpacesMutex;

unsigned long long Pyro::executeInternal() {
    auto startTime = std::chrono::system_clock::now();

    auto schema = relation_->getSchema();

    auto profilingContext = std::make_unique<ProfilingContext>(
            configuration_,
            relation_.get(),
            uccConsumer_,
            fdConsumer_,
            cachingMethod_,
            evictionMethod_,
            cachingMethodValue
            );

    std::function<bool(DependencyCandidate const&, DependencyCandidate const&)> launchPadOrder;
    if (configuration_.launchPadOrder == "arity") {
        launchPadOrder = DependencyCandidate::fullArityErrorComparator;
    } else if (configuration_.launchPadOrder == "error") {
        launchPadOrder = DependencyCandidate::fullErrorArityComparator;
    } else {
        throw std::runtime_error("Unknown comparator type");
    }

    int nextId = 0;
    if (configuration_.isFindKeys) {
        std::unique_ptr<DependencyStrategy> strategy;
        if (configuration_.uccErrorMeasure == "g1prime") {
            strategy = std::make_unique<KeyG1Strategy>(configuration_.maxUccError, configuration_.errorDev);
        } else {
            throw std::runtime_error("Unknown key error measure.");
        }
        searchSpaces_.push_back(std::make_unique<SearchSpace>(nextId++, std::move(strategy), schema, launchPadOrder));
    }
    if (configuration_.isFindFds) {
        for (auto& rhs : schema->getColumns()) {
            std::unique_ptr<DependencyStrategy> strategy;
            if (configuration_.uccErrorMeasure == "g1prime") {
                strategy = std::make_unique<FdG1Strategy>(rhs.get(), configuration_.maxUccError, configuration_.errorDev);
            } else {
                throw std::runtime_error("Unknown key error measure.");
            }
            searchSpaces_.push_back(std::make_unique<SearchSpace>(nextId++, std::move(strategy), schema, launchPadOrder));
        }
    }
    unsigned long long initTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();

    startTime = std::chrono::system_clock::now();
    unsigned int totalErrorCalcCount = 0;
    unsigned long long totalAscension = 0;
    unsigned long long totalTrickle = 0;
    double progressStep = 100.0 / searchSpaces_.size();

    const auto workOnSearchSpace = [this, &progressStep](
               std::list<std::unique_ptr<SearchSpace>>& searchSpaces,
               ProfilingContext* profilingContext, int id) {
        unsigned long long millis = 0;
        while (true) {
            auto threadStartTime = std::chrono::system_clock::now();
            std::unique_ptr<SearchSpace> polledSpace;
            {
                std::scoped_lock<std::mutex> lock(searchSpacesMutex);
                if (searchSpaces.empty()) {
                    break;
                }
                polledSpace = std::move(searchSpaces.front());
                searchSpaces.pop_front();
            }
            LOG(TRACE) << "Thread" << id << " got SearchSpace";
            polledSpace->setContext(profilingContext);
            polledSpace->ensureInitialized();
            polledSpace->discover();
            addProgress(progressStep);

            millis += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - threadStartTime).count();
        }
        //cout << "Thread" << id << " stopped working, ELAPSED TIME: " << millis << "ms.\n";
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < configuration_.parallelism; i++) {
        //std::thread();
        threads.emplace_back(workOnSearchSpace, std::ref(searchSpaces_), profilingContext.get(), i);
    }

    for (int i = 0; i < configuration_.parallelism; i++) {
        threads[i].join();
    }

    setProgress(100);
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
    LOG(INFO) << boost::format{"FdG1 error calculation: %1% ms"} % (FdG1Strategy::nanos_ / 1000000);
    LOG(INFO) << "Init time: " << initTimeMillis << "ms";
    LOG(INFO) << "Time: " << elapsed_milliseconds.count() << " milliseconds";
    LOG(INFO) << "Error calculation count: " << totalErrorCalcCount;
    LOG(INFO) << "Total ascension time: " << totalAscension << "ms";
    LOG(INFO) << "Total trickle time: " << totalTrickle << "ms";
    LOG(INFO) << "Total intersection time: "
              << util::PositionListIndex::micros / 1000 << "ms";
    LOG(INFO) << "HASH: " << PliBasedFDAlgorithm::fletcher16();
    return elapsed_milliseconds.count();
}


Pyro::Pyro(std::filesystem::path const &path, char separator, bool hasHeader, int seed, double maxError,
           unsigned int maxLHS, int parallelism) :
        PliBasedFDAlgorithm(path, separator, hasHeader),
        cachingMethod_(CachingMethod::COIN),
        evictionMethod_(CacheEvictionMethod::DEFAULT) {
    uccConsumer_ = [this](auto const& key) {
        this->discoverUCC(key);
    };
    fdConsumer_ = [this](auto const& fd) {
        this->discoverFD(fd);
        this->registerFD(fd.lhs_, fd.rhs_);
    };
    configuration_.seed = seed;
    configuration_.maxUccError = maxError;
    configuration_.maxUccError = maxError;
    configuration_.maxLHS = maxLHS;
    configuration_.parallelism = parallelism <= 0 ?
                                 std::thread::hardware_concurrency() :
                                 parallelism;
}

