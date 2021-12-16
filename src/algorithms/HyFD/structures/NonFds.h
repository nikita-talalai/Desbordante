#pragma once

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "NonFDList.h"

namespace HyFD {

    class NonFds {
    private:
        std::vector<std::set<boost::dynamic_bitset<>>> totalNonFds;
        // probably bad idea to store them separately, in case non FDs take a huge amount of memory
        NonFDList newNonFds;
    public:
        explicit NonFds(size_t numAttributes) : totalNonFds(numAttributes + 1), newNonFds(numAttributes + 1) {}

        void add(boost::dynamic_bitset<> fd);
        bool contains(boost::dynamic_bitset<> fd) const;
        size_t count() const;
        NonFDList moveOutNewNonFds();
        size_t numAttributes() const { return totalNonFds.size(); }
    };

}