#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>

namespace HyFD {

    class NonFDList {
    private:
        std::vector<std::vector<boost::dynamic_bitset<> >> fds;
        unsigned depth = 0;
    public:
        explicit NonFDList(size_t numAttributes) : fds(numAttributes) {}

        void add(boost::dynamic_bitset<> fd) {
            unsigned level = fd.count();

            fds[level].push_back(fd);
            if (level > depth) {
                depth = level;
            }
        }

        unsigned getDepth() const {
            return depth;
        }

        std::vector<boost::dynamic_bitset<>> const& getLevel(size_t level) const {
            return fds.at(level);
        }
    };

}