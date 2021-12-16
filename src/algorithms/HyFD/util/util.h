#pragma  once

#include "PositionListIndex.h"

namespace HyFD {

    inline bool isConstant(std::shared_ptr<util::PositionListIndex> pli) {

        if (pli->getSize() <= 1) {
            return false;

        }

        //TODO
        return true;
    }

}