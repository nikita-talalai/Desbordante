#include "Sampler.h"

namespace HyFD {

    void Sampler::runWindow(Sampler& sampler, Efficiency& efficiency, std::shared_ptr<util::PositionListIndex> pli) {

        size_t numAttributes = nonFds->numAttributes();
        size_t prevNumNonFds = nonFds->count();

        unsigned comps = 0;
        unsigned window = efficiency.getWindow();

        for (auto const& cluster: pli->getIndex()) {
            for (size_t i = 0; window < cluster.size() && i < cluster.size() - window; ++i) {
                int pivotId = cluster[i];
                int partnerId = cluster[i + window - 1];

                boost::dynamic_bitset<> equalAttrs(numAttributes);
                sampler.match(equalAttrs, pivotId, partnerId);
                nonFds->add(equalAttrs);

                comps++;
            }
        }

        size_t numNewViolations = nonFds->count() - prevNumNonFds;

        efficiency.addViolations(numNewViolations);
        efficiency.addComparisons(comps); // TODO: mb just increment inside loop ^, or use an advanced formula
    }

    NonFDList
    Sampler::getNonFDCandidates(const std::vector<std::pair<size_t, size_t>> &comparisonSuggestions) {
        size_t numAttributes = plis.size();

        boost::dynamic_bitset<> equalAttrs(numAttributes);

        for (auto [firstId, secondId]: comparisonSuggestions) {
            match(equalAttrs, firstId, secondId);

            //check ????
            nonFds->add(equalAttrs);
        }

        if (efficiencyQueue.empty()) {

            //TODO: SORT??????

            for (size_t attr = 0; attr < numAttributes; ++attr) {
                Efficiency efficiency(attr);

                runWindow(*this, efficiency, plis[attr]);

                if (efficiency.calcEfficiency() > 0) {
                    efficiencyQueue.push(efficiency);
                }
            }
        } else {
            efficiencyThreshold = efficiencyThreshold / 2;
        }

        while (!efficiencyQueue.empty()) {
            Efficiency bestEff = efficiencyQueue.top();
            efficiencyQueue.pop();

            if (bestEff.calcEfficiency() < efficiencyThreshold) {
                break;
            }

            bestEff.incrementWindow();
            runWindow(*this, bestEff, plis[bestEff.getAttr()]);
        }

        return nonFds->moveOutNewNonFds();
    }

    void Sampler::match(boost::dynamic_bitset<>& bSet, size_t firstRecordId, size_t secondRecordId) {
        assert(firstRecordId < compressedRecords.size() && secondRecordId < compressedRecords.size());

        for (size_t i = 0; i < compressedRecords[0].size(); ++i) {
            if (compressedRecords[firstRecordId][i] == compressedRecords[secondRecordId][i]) {
                bSet.set(i);
            }
        }
    }

}