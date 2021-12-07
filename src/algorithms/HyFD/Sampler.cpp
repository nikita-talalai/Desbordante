#include "Sampler.h"

namespace HyFD {

    void runWindow(Sampler &sampler, Efficiency &efficiency,
                   std::shared_ptr<util::PositionListIndex> pli, std::shared_ptr<nonFDList> nonFds) {

        //efficiency.incrementWindow();
        size_t numAttributes = nonFds->getNumAttributes();

        size_t prevNumNonFds = nonFds->getSize();


        int comps = 0;

        int window = efficiency.getWindow();

        boost::dynamic_bitset<> equalAttrs(numAttributes);

        for (auto &cluster: pli->getIndex()) {
            for (size_t i = 0; i < cluster.size() - window; ++i) {
                int pivot = cluster[i];

                int partner = cluster[i + window - 1];

                sampler.match(equalAttrs, pivot, partner);

                comps++;
            }

        }

        int results = nonFds->getSize() - prevNumNonFds;

        efficiency.setResults(results);
        efficiency.setComps(comps);


    }

    std::shared_ptr<nonFDList>
    Sampler::getNonFDCandidate(const std::vector<std::pair<size_t, size_t>> &comparisonSuggestions) {
        size_t numAttributes = plis.size();

        std::shared_ptr<nonFDList> newNonFDs(new nonFDList(numAttributes));

        boost::dynamic_bitset<> equalAttrs(numAttributes);

        for (auto sug: comparisonSuggestions) {
            match(equalAttrs, sug.first, sug.second);

            //check ????
            fdSet->add(equalAttrs);
        }

        if (efficiencyQueue.empty()) {

            //SORT??????

            for (size_t attr = 0; attr < numAttributes; ++attr) {
                Efficiency efficiency(attr);

                runWindow(*this, efficiency, plis[attr], newNonFDs);

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

            runWindow(*this, bestEff, plis[bestEff.getAttr()], newNonFDs);
        }

        return newNonFDs;

    }

    void Sampler::match(boost::dynamic_bitset<> &bSet, int lhs, int rhs) {
        for (size_t i = 0; i < compressedRecords[0].size(); ++i) {
            if (compressedRecords[lhs][i] == compressedRecords[rhs][i]) {
                bSet.set(i);
            }
        }
    }

}