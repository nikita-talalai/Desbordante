#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include <iostream>
#include <memory>

//TODO: to .cpp or smth; tests

namespace HyFD::FDTree {

    class FDTreeVertex;

    using LhsPair = std::pair<std::shared_ptr<FDTreeVertex>, boost::dynamic_bitset<>>;

    class FDTreeVertex : private std::enable_shared_from_this<FDTreeVertex> {
    private:
        std::vector<std::shared_ptr<FDTreeVertex>> childs;
        boost::dynamic_bitset<> attributes;
        boost::dynamic_bitset<> fds;
        size_t numAttributes;

        bool containsChildren = false;

    public:
        explicit FDTreeVertex(size_t numAttributes) :
                attributes(numAttributes),
                numAttributes(numAttributes) {}

        size_t numAttrs() const {
            return numAttributes;
        }

        void setFds(boost::dynamic_bitset<> newFds) {
            fds = newFds;
        }

        boost::dynamic_bitset<> getAttributes() const {
            return attributes;
        }

        void addAttribute(size_t pos) {
            attributes.set(pos);
        }

        void removeAttribute(size_t pos) {
            attributes.reset(pos);
        }

        void addFd(size_t pos) {
            fds.set(pos);
        }

        void removeFd(size_t pos) {
            fds.reset(pos);
        }

        void removeAttr(size_t pos) {
            attributes.reset(pos);
        }

        bool isFd(size_t pos) const {
            return fds.test(pos);
        }

        bool isAttribute(size_t pos) const {
            return fds.test(pos);
        }

        void addChild(size_t pos) {
            containsChildren = true;
            if (childs.empty()) {
                childs.resize(numAttributes);
            }

            if (!containsChildAt(pos)) {
                childs[pos] = std::make_shared<FDTreeVertex>(numAttributes);
            }
        }

        bool anyRhsAttr() const {
            return attributes.count() > 0;
        }

        // TODO: shared_ptr<const>
        std::shared_ptr<FDTreeVertex> getChild(size_t pos) const {
            return childs.at(pos);
        }

        bool containsChildAt(size_t pos) const {
            return childs.at(pos) != nullptr;
        }

        bool hasChildren() const {
            return containsChildren;
        }

        void getLevelRecursive(unsigned targetLevel, unsigned curLevel, boost::dynamic_bitset<> lhs, std::vector<LhsPair>& vertices) {

            if (curLevel == targetLevel) {
                vertices.emplace_back(shared_from_this(), lhs);
                return;
            }

            if (!hasChildren()) {
                return;
            }

            // TODO: maybe use 'attributes' to visit children?
            for (size_t i = 0; i < numAttributes; ++i) {

                if (containsChildAt(i)) {
                    lhs.set(i);

                    childs[i]->getLevelRecursive(targetLevel, curLevel + 1, lhs, vertices);

                    lhs[i] = false;
                }
            }
        }

        void getFdAndGeneralsRecursive(dynamic_bitset<> lhs, boost::dynamic_bitset<> curLhs,
                                       size_t rhs, size_t curBit, std::vector<boost::dynamic_bitset<>>& result) {
            if (isFd(rhs)) {
                result.push_back(curLhs);
            }

            if (!hasChildren()) {
                return;
            }

            for (auto attrId = curBit; attrId < lhs.size(); attrId = lhs.find_next(attrId)) {
                if (containsChildAt(attrId) && childs[attrId]->isAttribute(rhs)) {

                    curLhs.set(attrId);
                    childs[attrId]->getFdAndGeneralsRecursive(lhs, curLhs, rhs, lhs.find_next(attrId), result);
                    curLhs.reset(attrId);
                }
            }
        }

        bool findFdOrGeneralRecursive(dynamic_bitset<> lhs, size_t rhs, size_t curBit) {

            if (curBit >= lhs.size()) {
                return false;
            }

            // TODO: probably 'childs[curBit]->isFd(rhs)' is wrong
            if (isFd(rhs) || (containsChildAt(curBit) && childs[curBit]->isFd(rhs)
                              && childs[curBit]->findFdOrGeneralRecursive(lhs, rhs, lhs.find_next(curBit)))) {
                return true;
            }

            return findFdOrGeneralRecursive(lhs, rhs, lhs.find_next(curBit));
        }
    };

    class FDTree {
    private:
        std::shared_ptr<FDTreeVertex> root;

    public:
        explicit FDTree(size_t numAttributes) : root(std::make_shared<FDTreeVertex>(numAttributes)) {}

        size_t numAttributes() const {
            return root->numAttrs();
        }

        // TODO: ummm... recursive calls? depth > 1??
        void addFD(boost::dynamic_bitset<> lhs, size_t rhs) {
            std::shared_ptr<FDTreeVertex> curNode = root;
            root->addAttribute(rhs);

            // bit < ...
            for (int bit = lhs.find_first(); bit >= 0; bit = lhs.find_next(bit + 1)) {

                curNode->addChild(bit);
                curNode = root->getChild(bit);

                curNode->addAttribute(rhs);
            }
        }


        //TODO: same
        bool containFD(boost::dynamic_bitset<> lhs, size_t rhs) {
            std::shared_ptr<FDTreeVertex> curNode = root;

            if (!root->hasChildren()) {
                return false;
            }

            for (int bit = lhs.find_first(); bit >= 0; bit = lhs.find_next(bit)) {

                if (!curNode->containsChildAt(bit)) {
                    return false;
                }

                curNode = curNode->getChild(bit);
            }

            return curNode->isFd(rhs);

        }

        std::shared_ptr<FDTreeVertex> getRoot() {
            return root;
        }

        void remove(boost::dynamic_bitset<> lhs, size_t rhs) {

            auto curNode = getRoot();

            for (int attr = lhs.find_first();; attr = lhs.find_next(attr + 1)) {
                if (!curNode->containsChildAt(attr)) {
                    break;
                }

                curNode = curNode->getChild(attr);
            }

            root->removeAttribute(rhs);
            root->removeFd(rhs);

            bool foundRhs = false;

            for (int attr = 0, numAttr = numAttributes(); attr < numAttr; attr++) {
                if (root->containsChildAt(attr) && root->getChild(attr)->isAttribute(rhs)) {
                    foundRhs = true;

                    break;
                }
            }

            if (!foundRhs) {
                root->removeAttribute(rhs);
            }

            if (root->anyRhsAttr()) {
                //release;
            }

        }

        std::vector<boost::dynamic_bitset<>> getFdAndGenerals(boost::dynamic_bitset<> targetLhs, size_t targetRhs) {
            assert(targetLhs.count() != 0);

            std::vector<boost::dynamic_bitset<>> result;
            const boost::dynamic_bitset<> emptyLhs(numAttributes());
            const size_t startingBit = targetLhs.find_first();

            root->getFdAndGeneralsRecursive(targetLhs, emptyLhs, targetRhs, startingBit, result);

            return result;
        }

        bool findFdOrGeneral(boost::dynamic_bitset<> lhs, size_t rhs) {
            return root->findFdOrGeneralRecursive(lhs, rhs, lhs.find_first());
        }

        std::vector<LhsPair> getLevel(unsigned targetLevel) {

            boost::dynamic_bitset<> emptyLhs(numAttributes());

            std::vector<LhsPair> vertices;
            root->getLevelRecursive(targetLevel, 0, emptyLhs, vertices);

            return vertices;
        }
    };

}
