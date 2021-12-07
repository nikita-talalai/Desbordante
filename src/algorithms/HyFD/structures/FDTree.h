#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include <iostream>
#include <memory>


namespace HyFD {

    class FDTreeVertex;

    using LhsPair = std::pair<std::shared_ptr<FDTreeVertex>, boost::dynamic_bitset<>>;

    class FDTreeVertex {
    private:
        std::vector<std::shared_ptr<FDTreeVertex>> childs;
        boost::dynamic_bitset<> attributes;
        boost::dynamic_bitset<> fds;
        size_t numAttributes;

        bool containChilds;


    public:
        explicit FDTreeVertex(size_t numAttributes) :
                attributes(numAttributes),
                numAttributes(numAttributes) {}

        size_t numAttrs() const {
            return numAttributes;
        }

        void setFds(const boost::dynamic_bitset<> newFds) {
            fds = newFds;
        }


        boost::dynamic_bitset<> getAttributes() const {
            return attributes;
        }


        void addAttributes(size_t pos) {
            attributes.set(pos);
        }

        void removeAttributes(size_t pos) {
            attributes[pos] = false;
        }

        void addFds(size_t pos) {
            fds.set(pos);
        }


        void remove(size_t pos) {
            fds[pos] = false;
        }

        void removeAttr(size_t pos) {
            attributes[pos] = false;
        }

        bool getFDs(size_t pos) const {
            return fds.test(pos);
        }


        bool getAttributes(size_t pos) const {
            return fds.test(pos);
        }

        void addChild(size_t pos) {

            containChilds = true;
            if (childs.empty()) {
                childs.resize(numAttributes);
            }

            if (!containChild(pos)) {
                childs[pos] = std::make_shared<FDTreeVertex>(numAttributes);
            }
        }

        bool anyRhsAttr() const {
            return attributes.count() > 0;
        }

        std::shared_ptr<FDTreeVertex> getChild(size_t pos) const {
            return childs.at(pos);
        }

        void getLevelRecursive(int level, int curLevel, boost::dynamic_bitset<> lhs, std::vector<LhsPair> &vertexs) {

            if (curLevel == level) {

                vertexs.push_back({std::shared_ptr<FDTreeVertex>(this), lhs});
                return;
            }

            if (!anyChild()) {
                return;
            }


            for (size_t i = 0; i < numAttributes; ++i) {

                if (containChild(i)) {
                    lhs.set(i);

                    childs[i]->getLevelRecursive(level, curLevel + 1, lhs, vertexs);

                    lhs[i] = 0;
                }
            }

        }


        void getFdAndGeneralslRecursive(const dynamic_bitset<> lhs, boost::dynamic_bitset<> &curLhs,
                                        int rhs, int curBit, std::vector<boost::dynamic_bitset<>> &result) {

            if (getFDs(rhs)) {

                result.push_back(curLhs);
            }

            if (!anyChild()) {

                return;
            }

            for (int attr = curBit; attr >= 0; attr = lhs.find_next(attr + 1)) {
                if (containChild(attr)) {
                    if (childs[attr]->getAttributes(rhs)) {

                        curLhs.set(attr);
                        childs[attr]->getFdAndGeneralslRecursive(lhs, curLhs, rhs, lhs.find_next(attr + 1), result);

                        curLhs[attr] = 0;
                    }
                }
            }


        }

        bool findFdOrGeneralRecursive(const dynamic_bitset<> lhs, int rhs, int curBit) {

            if (curBit < 0) {
                return false;
            }
            if (getFDs(rhs)) {
                return true;
            }

            if (containChild(curBit) && childs[curBit]->getFDs(rhs)) {
                if (childs[curBit]->findFdOrGeneralRecursive(lhs, rhs, lhs.find_next(curBit + 1))) {
                    return true;
                }
            }

            return findFdOrGeneralRecursive(lhs, rhs, lhs.find_next(curBit + 1));

        }

        bool containChild(size_t pos) const {
            return bool(childs.at(pos));
        }

        bool anyChild() const {
            return containChilds;
        }

    };

    using LhsPair = std::pair<std::shared_ptr<FDTreeVertex>, boost::dynamic_bitset<>>;

    class FDTree {
    private:


        std::shared_ptr<FDTreeVertex> root;
    public:

        size_t numAttributes() const {
            return root->numAttrs();
        }

        void addFD(boost::dynamic_bitset<> lhs, int rhs) {
            std::shared_ptr<FDTreeVertex> curNode = root;
            root->addAttributes(rhs);


            for (int bit = lhs.find_first(); bit >= 0; bit = lhs.find_next(bit + 1)) {

                curNode->addChild(bit);
                curNode = root->getChild(bit);

                curNode->addAttributes(rhs);
            }
        }


        bool containFD(boost::dynamic_bitset<> lhs, int rhs) {
            std::shared_ptr<FDTreeVertex> curNode = root;

            if (!root->anyChild()) {
                return false;
            }

            for (int bit = lhs.find_first(); bit >= 0; bit = lhs.find_next(bit)) {

                if (!curNode->containChild(bit)) {
                    return false;
                }

                curNode = curNode->getChild(bit);
            }

            return curNode->getFDs(rhs);

        }

        std::shared_ptr<FDTreeVertex> getRoot() {
            return root;
        }

        void remove(boost::dynamic_bitset<> lhs, size_t rhs) {

            auto curNode = getRoot();

            for (int attr = lhs.find_first();; attr = lhs.find_next(attr + 1)) {
                if (!curNode->containChild(attr)) {
                    break;
                }

                curNode = curNode->getChild(attr);
            }

            root->removeAttributes(rhs);
            root->remove(rhs);

            bool foundRhs = false;

            for (int attr = 0, numAttr = numAttributes(); attr < numAttr; attr++) {
                if (root->containChild(attr) && root->getChild(attr)->getAttributes(rhs)) {
                    foundRhs = true;

                    break;
                }
            }

            if (!foundRhs) {
                root->removeAttributes(rhs);
            }

            if (root->anyRhsAttr()) {
                //release;
            }

        }

        std::vector<boost::dynamic_bitset<>> getFdAndGenerals(boost::dynamic_bitset<> lhs, int rhs) {
            std::vector<boost::dynamic_bitset<>> result;

            boost::dynamic_bitset<> curLhs;
            root->getFdAndGeneralslRecursive(lhs, curLhs, rhs, lhs.find_first(), result);

            return result;
        }

        bool findFdOrGeneral(boost::dynamic_bitset<> lhs, int rhs) {
            return root->findFdOrGeneralRecursive(lhs, rhs, lhs.find_first());
        }

        std::vector<LhsPair> getLevel(int level) {

            boost::dynamic_bitset<> lhs(numAttributes());

            std::vector<LhsPair> vertexs;
            root->getLevelRecursive(level, 0, lhs, vertexs);


            return vertexs;
        }

    };

}
