#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <boost/dynamic_bitset.hpp>

namespace HyFD {

    class nonFDList {
    private:
        size_t numAttributes;
        std::vector<std::vector<boost::dynamic_bitset<> >> fds;
        size_t size;
        int depth;
    public:
        explicit nonFDList(int numAttributes) :
                numAttributes(numAttributes) {

            fds.resize(numAttributes);
        }

        void add(boost::dynamic_bitset<> fd) {
            int level = fd.count();

            fds[level].push_back((fd));
            size++;
            if (level > depth) {
                depth = level;
            }


        }

        size_t getNumAttributes() const {
            return numAttributes;
        }


        int getDepth() const {
            return depth;
        }

        size_t getSize() const {
            return size;
        }

        const std::vector<boost::dynamic_bitset<>> &getLevel(size_t level) const {
            return fds.at(level);
        }
    };

}