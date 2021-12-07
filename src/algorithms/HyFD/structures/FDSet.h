#pragma  once

#include <vector>
#include <iostream>
#include <memory>
#include <set>
#include <boost/dynamic_bitset.hpp>

namespace HyFD {

    class FDSet {
    private:
//        int numAttributes;
        std::vector<std::set<boost::dynamic_bitset<> >> fds;
        int depth;
    public:
        explicit FDSet(size_t numAttributes) /*: TODO: remove
                numAttributes(numAttributes)*/ {

            fds.resize(numAttributes);
        }

        void add(boost::dynamic_bitset<> fd) {
            int level = fd.count();

            fds[level].insert(fd);

            if (level > depth) {
                depth = level;
            }


        }

        bool contain(boost::dynamic_bitset<> fd) const {
            size_t level = fd.count();

            return fds[level].count(fd);
        }

        int getDepth() const {
            return depth;
        }

        size_t size() const {
            return fds.size();
        }


    };

}