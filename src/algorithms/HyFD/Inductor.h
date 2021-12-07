#pragma once

#include <vector>
#include <iostream>

#include "structures/nonFDList.h"
#include "structures/FDSet.h"
#include "HyFD.h"
#include "structures/FDTree.h"

namespace HyFD {

    class Inductor {
    private:
        std::shared_ptr<FDTree> tree;
    public:

        void update(std::shared_ptr<nonFDList> fdlist);

        void specialize(boost::dynamic_bitset<> lhs, int rhs);

    };

}
