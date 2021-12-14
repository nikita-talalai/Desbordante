#pragma  once

#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <unordered_set>

#include "structures/NonFDList.h"
#include "structures/NonFds.h"
#include "HyFD.h"
#include "structures/FDTree.h"
#include "util/util.h"

namespace HyFD {

    using LhsPair = FDTree::LhsPair;

    struct FD {
        boost::dynamic_bitset<> lhs;
        int rhs;
    };

    struct FDValidations {

        std::vector<FD> invalidFDs;
        std::vector<std::pair<size_t, size_t>> comparisonSuggestions;

        int countValidations;
        int countIntersections;


        void add(const FDValidations &other) {
            invalidFDs.insert(invalidFDs.end(), other.invalidFDs.begin(), other.invalidFDs.end());

            comparisonSuggestions.insert(comparisonSuggestions.end(),
                                         other.comparisonSuggestions.begin(), other.comparisonSuggestions.end());

            countIntersections += other.countIntersections;
            countValidations += other.countValidations;
        }

    };


    class Validator {
    private:
        std::shared_ptr<FDTree::FDTree> fds;
        std::shared_ptr<NonFds> fdList;

        std::vector<std::shared_ptr<util::PositionListIndex>> plis; //TODO: initialize lol
        double efficiencyThreshold;

        int currentLevelNumber;
        std::vector<std::vector<int>> compressedRecords;


    public:

        FDValidations validateSeq(const std::vector<LhsPair> &vertexs) {

            FDValidations result;
            for (auto &vertex: vertexs) {
                result.add(std::move(getValidations(vertex)));
            }

            return result;
        }

        FDValidations getValidations(const LhsPair &lhsPair);

        int getLevelNum() const {
            return currentLevelNumber;
        }


        std::vector<std::pair<size_t, size_t>> validate();
    };

}