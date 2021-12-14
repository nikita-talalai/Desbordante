#pragma once

#include <vector>
#include <iostream>

#include "structures/NonFDList.h"
#include "structures/NonFds.h"
#include "HyFD.h"
#include "structures/FDTree.h"

namespace HyFD {

    class Inductor {
    private:
        std::shared_ptr<FDTree::FDTree> tree;
    public:

        void update(NonFDList const& fdlist);

        void specialize(boost::dynamic_bitset<> lhsBits, size_t rhsId);

    };

}
