#include "Inductor.h"

namespace HyFD {

    void Inductor::update(NonFDList const& fdlist) {
        assert(fdlist.getDepth() != 0);
        unsigned maxLevel = fdlist.getDepth(); // - 1;
        //sort????????????? - done by level layout
        bool processedZeroLevel = false;
        for (unsigned level = maxLevel; !processedZeroLevel; processedZeroLevel = (level == 0), level--) { // TODO: как бы проитерироваться

            for (const auto &lhs: fdlist.getLevel(level)) {

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
