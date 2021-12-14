#include "Inductor.h"

namespace HyFD {

    void Inductor::update(NonFDList const& fdlist) {
        assert(fdlist.getDepth() != 0);
        const unsigned maxLevel = fdlist.getDepth(); // - 1;

        bool processedZeroLevel = false;
        // TODO: beautify the loop
        for (unsigned level = maxLevel; !processedZeroLevel; processedZeroLevel = (level == 0), level--) {

            for (auto const& lhsBits: fdlist.getLevel(level)) {

                auto rhsBits = lhsBits;
                rhsBits.flip();

                for (size_t rhsId = rhsBits.find_first();
                        rhsId < rhsBits.size();
                        rhsId = rhsBits.find_next(rhsId)) {
                    specialize(lhsBits, rhsId);
                }
            }
        }
    }

    void Inductor::specialize(boost::dynamic_bitset<> lhsBits, size_t rhsId) {
        auto invalidLhss = tree->getFdAndGenerals(lhsBits, rhsId);

        for (auto invalidLhsBits: invalidLhss) {
            tree->remove(invalidLhsBits, rhsId);

            for (size_t i = 0; i < tree->numAttributes(); ++i) {
                if (i == rhsId) {
                    continue;
                }
                if (invalidLhsBits.test(i)) {
                    continue;
                }

                invalidLhsBits.set(i);

                if (tree->findFdOrGeneral(invalidLhsBits, rhsId)) {
                    invalidLhsBits.reset(i);
                    continue;
                }

                tree->addFD(invalidLhsBits, rhsId);
                invalidLhsBits.reset(i);
            }
        }
    }

}
