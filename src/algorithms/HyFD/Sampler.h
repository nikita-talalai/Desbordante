#pragma once 

#include "HyFD.h"
#include "structures/NonFDList.h"
#include "structures/NonFds.h"
#include "structures/FDTree.h"

#include <memory>
#include <vector>
#include <queue>
#include <memory>

namespace HyFD {

using PLIS = std::vector<std::shared_ptr<util::PositionListIndex>>;
class Efficiency{
private:
    size_t columnId;

    //TODO: num* dependent on windowSize?
    unsigned numViolations = 0;
    unsigned numComparisons = 0;
    unsigned window = 2;
public:
    explicit Efficiency(size_t columnId) : columnId(columnId) {}

    double calcEfficiency() const {
        if (numComparisons == 0) {
            return 0.0;
        }

        return 1.00 * numViolations / numComparisons;
    }

    void incrementWindow() {
        window++;
    }

    void addComparisons(unsigned numNewComparisons) {
        numComparisons += numNewComparisons;
    }

    void addViolations(unsigned numNewViolations) {
        numViolations += numNewViolations;
    }

    size_t getAttr() const {
        return columnId;
    }

    unsigned getWindow() const {
        return window;
    }
};

inline bool operator<(Efficiency const& lhs, Efficiency const& rhs){
    return lhs.calcEfficiency() < rhs.calcEfficiency();
}

class Sampler {
private:

    PLIS plis;
    std::vector<std::vector<int>> compressedRecords;
    std::priority_queue<Efficiency> efficiencyQueue;

    double efficiencyThreshold; //TODO: out into HyFD conf or smth
    std::shared_ptr<NonFds> nonFds;

public:
    explicit Sampler(PLIS const& plis, std::vector<std::vector<int>>&& pliRecords):
            plis(plis), compressedRecords(std::move(pliRecords)) {
        efficiencyThreshold = 0.01;
        nonFds = std::make_shared<NonFds>(plis.size());
    }

    NonFDList getNonFDCandidates(std::vector<std::pair<size_t, size_t>> const& comparisonSuggestions);

    void match(boost::dynamic_bitset<>& bSet, size_t firstRecordId, size_t secondRecordId);
    void runWindow(Sampler& sampler, Efficiency& efficiency, std::shared_ptr<util::PositionListIndex> pli);
};


}


