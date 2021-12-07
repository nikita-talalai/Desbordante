#include "Inductor.h"

namespace HyFD {

    void Inductor::update(std::shared_ptr<nonFDList> fdlist) {
        int maxLevel = fdlist->getDepth() - 1;
        //sort?????????????
        for (int level = maxLevel; level >= 0; level--) {

            for (const auto &lhs: fdlist->getLevel(level)) {

                auto rhss = lhs;

                rhss.flip();

                for (int rhs = rhss.find_first(); rhs >= 0; rhs = rhss.find_next(rhs)) {
                    specialize(lhs, rhs);
                }
            }
        }
    }

    void Inductor::specialize(boost::dynamic_bitset<> lhs, int rhs) {
        auto invalidLhss = tree->getFdAndGenerals(lhs, rhs);

        for (auto invalidLhs: invalidLhss) {
            tree->remove(invalidLhs, rhs);

            for (size_t i = 0; i < tree->numAttributes(); ++i) {
                if (i == size_t(rhs)) {
                    continue;
                }
                if (invalidLhs.test(i)) {
                    continue;
                }

                invalidLhs.set(i);

                if (tree->findFdOrGeneral(invalidLhs, rhs)) {
                    continue;
                }

                tree->addFD(invalidLhs, rhs);

            }
        }

    }

}
