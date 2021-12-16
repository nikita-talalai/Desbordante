#include "HyFD.h"

#include "Sampler.h"
#include "Inductor.h"
#include "Validator.h"

namespace HyFD {

    unsigned long long HyFD::executeInternal() {
//        const auto schema = getRelation().getSchema();

        PLIS plis;
        std::transform(relation_->getColumnData().begin(), relation_->getColumnData().end(),
                       std::back_inserter(plis), [](auto& columnData) { return columnData.getPLIOwnership(); });

        size_t numColumns = getRelation().getNumColumns();
        size_t numRows = getRelation().getNumRows();

        std::vector<std::vector<int>> invertedPlis;
        for (auto &pli: plis) {
            size_t clusterId = 0;
            std::vector<int> current(numRows, -1);
            for (const auto &cluster: pli->getIndex()) {
                for (int value: cluster) {
                    current[value] = clusterId;
                }
                clusterId++;
            }
            invertedPlis.push_back(std::move(current));
        }

        std::vector<std::vector<int>> pliRecords(numRows, std::vector<int> (numColumns));

        for (size_t i = 0; i < numRows; ++i) {
            for (size_t j = 0; j < numColumns; ++j) {
                pliRecords[i][j] = invertedPlis[j][i];
            }
        }

        Sampler sampler(plis, std::move(pliRecords));
        Inductor inductor;
        Validator validator;

        std::vector<std::pair<size_t, size_t>> comparisonSuggestions;

        while (true) {
            const auto fdList = sampler.getNonFDCandidates(comparisonSuggestions);

            inductor.update(fdList);

            comparisonSuggestions = validator.validate();

            if (comparisonSuggestions.empty()) {
                break;
            }
        }
        return 0;
    }

}