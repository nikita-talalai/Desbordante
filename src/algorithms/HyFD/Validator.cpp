#include "Validator.h"

namespace HyFD {

    FDValidations Validator::getValidations(const LhsPair &lhsPair) {
        FDValidations result;

        auto vertex = lhsPair.first;

        auto lhs = vertex->getAttributes();
        auto rhs = lhsPair.second;

        size_t rhsCount = rhs.count();


        if (rhsCount == 0) {
            return result;
        }

        result.countValidations = rhsCount;


        if (getLevelNum() == 0) {


            result.countIntersections = rhs.count();

            for (int attr = rhs.find_first(); attr >= 0; attr = rhs.find_next(attr + 1)) {

                if (!isConstant(plis[attr])) {

                    vertex->removeAttribute(attr);
                    result.invalidFDs.push_back({lhs, attr});
                }
            }

            return result;

        }


        if (getLevelNum() == 1) {
            result.countIntersections = rhs.count();

            int firstAttr = rhs.find_first();

            for (int attr = firstAttr; attr >= 0; attr = rhs.find_next(attr + 1)) {

                for (const auto &cluster: plis[firstAttr]->getIndex()) {
                    int clusterId = compressedRecords[cluster[0]][attr];

                    if (clusterId >= 0) {
                        for (int id: cluster) {

                            if (compressedRecords[id][attr] != clusterId) {

                                vertex->removeAttribute(attr);
                                result.invalidFDs.push_back({lhs, attr});

                                break;
                            }
                        }
                    } else {

                        vertex->removeAttribute(attr);
                        result.invalidFDs.push_back({lhs, attr});
                        break;
                    }

                }


            }
            return result;
        }

        int firstAttr = rhs.find_first();

        result.countIntersections++;

        lhs[firstAttr] = 0;

        boost::dynamic_bitset<> validRhss = rhs;

        std::vector<int> attrIdInv(compressedRecords[0].size());
        std::vector<int> attrId;

        for (int attr = rhs.find_first(); attr >= 0; attr = rhs.find_next(attr + 1)) {
            attrIdInv[attr] = attrId.size();
            attrId.push_back(attr);
        }

        auto cmp = [](const std::pair<std::vector<int>, int> &lhsParam,
                      const std::pair<std::vector<int>, int> &rhsParam) {
            if (lhsParam.second == rhsParam.second) {
                return lhsParam.first < rhsParam.first;
            } else {
                return lhsParam.second < rhsParam.second;
            }
        };

        bool refinedIsEmpty = false;

        for (auto const& cluster: plis[firstAttr]->getIndex()) {
            if (refinedIsEmpty) {
                break;
            }

            auto subClusters = std::set<std::pair<std::vector<int>, int>, decltype(cmp)>(cmp);

            for (int id:  cluster) {

                std::vector<int> subCluster;
                for (int attr = lhs.find_first(); attr >= 0; attr = lhs.find_next(attr + 1)) {
                    if (compressedRecords[id][attr] != -1) {
                        subCluster.push_back(compressedRecords[id][attr]);

                    } else {
                        break;
                    }
                }

                if (subCluster.size() < lhs.count()) {
                    continue;
                }

                auto iter = subClusters.find({subCluster, id});

                if (iter != subClusters.end()) {

                    auto[vec, record] = *iter;

                    for (int attr = validRhss.find_first(); attr >= 0; attr = validRhss.find_next(attr + 1)) {
                        int value = compressedRecords[id][attr];

                        if (value == -1 || value != vec[attrIdInv[attr]]) {

                            validRhss[attr] = 0;
                            result.comparisonSuggestions.push_back({id, record});


                            if (validRhss.count() == 0) {
                                refinedIsEmpty = true;

                                break;
                            }
                        } else {
                            std::vector<int> newSubCluster;

                            for (size_t i = 0; i < rhs.count(); ++i) {
                                newSubCluster[i] = compressedRecords[id][attrId[i]];
                            }

                            subClusters.insert(std::make_pair(std::move(newSubCluster), id));

                        }
                    }

                }

            }


        }
        //????????
        rhs &= validRhss;

        vertex->setFds(validRhss);

        for (int attr = rhs.find_first(); attr >= 0; attr = rhs.find_next(attr + 1)) {
            result.invalidFDs.push_back({lhs, attr});
        }

        return result;

    }

    std::vector<std::pair<size_t, size_t>> Validator::validate() {

        size_t numAttributes = plis.size();

        std::vector<LhsPair> curLevelVertexs;

        if (currentLevelNumber) {
            curLevelVertexs = fds->getLevel(currentLevelNumber);
        } else {

            curLevelVertexs.push_back({fds->getRoot(), boost::dynamic_bitset<>(numAttributes)});
        }

        std::vector<std::pair<size_t, size_t>> comparisonSuggestions;
        while (curLevelVertexs.size()) {

            auto result = validateSeq(curLevelVertexs);

            comparisonSuggestions.insert(comparisonSuggestions.begin(),
                                         result.comparisonSuggestions.begin(), result.comparisonSuggestions.end());

            std::vector<LhsPair> nextLevel;

            for (auto &lhsPair:  curLevelVertexs) {
                auto vertex = lhsPair.first;

                if (!vertex->hasChildren()) {

                    continue;
                }

                auto lhs = lhsPair.second;
                //???????????
                for (size_t i = 0; i < numAttributes; ++i) {
                    auto child = vertex->getChild(i);

                    //TODO check cond
                    boost::dynamic_bitset<> childLhs = lhs;
                    childLhs.set(i);
                    nextLevel.push_back({child, childLhs});


                }
            }

            for (auto invalidFd: result.invalidFDs) {
                auto lhs = invalidFd.lhs;
                size_t rhs = invalidFd.rhs;

                for (size_t attr = 0; attr < numAttributes; ++attr) {

                    if (lhs.test_set(attr) || rhs == attr ||
                        fds->findFdOrGeneral(lhs, rhs) || fds->containFD(lhs, rhs)) {

                        continue;
                    }

                    boost::dynamic_bitset<> newLhs = lhs;
                    newLhs.set(attr);

                    if (fds->findFdOrGeneral(newLhs, rhs)) {
                        continue;
                    }

                    //TODO
                    //auto  child = totalNonFds->addFD(newLhs, rhs);
                    //nextLevel.push_back({child, newLhs});
                }
            }

            currentLevelNumber++;

            if (result.invalidFDs.size() > efficiencyThreshold * result.countValidations) {
                return comparisonSuggestions;
            }


            return {};
        }

        return {};
    }
}