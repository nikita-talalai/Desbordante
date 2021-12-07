#pragma once 

#include "HyFD.h"
#include "structures/nonFDList.h"
#include "structures/FDSet.h"
#include "structures/FDTree.h"

#include <memory>
#include <vector>
#include <queue>
#include <memory>

namespace HyFD {

using PLIS = std::vector<std::shared_ptr<util::PositionListIndex>>;
class Efficiency{
private:
    size_t attr;
    int results;

    int comps;
    int window;
public:
    explicit Efficiency(size_t attr):
        attr(attr)
    {
        window = 2;
    }

    double calcEfficiency() const {
        if (comps == 0) {
            return  0.0;
        }

        return 1.00 * results / comps;
    }

    void incrementWindow() {
        window++;
    }

    void setComps(int newComps){
        comps = newComps;
    }

    void setResults(int newResults){
        results = newResults;
    }


    size_t getAttr() const {
        return attr;
    }

    int getWindow() const {
        return window;
    }


};

inline  bool operator < (const Efficiency &lhs, const Efficiency & rhs){
    return  lhs.calcEfficiency() < rhs.calcEfficiency();
}

class Sampler{
private:

    PLIS plis;
    std::priority_queue<Efficiency> efficiencyQueue;

    double efficiencyThreshold; //TODO: out into HyFD conf or smth
    std::vector<std::vector<int>> compressedRecords;
    std::shared_ptr<FDSet> fdSet;

public:
    explicit Sampler(const PLIS &plis):
        plis(plis)

    {
        efficiencyThreshold = 0.01;
        fdSet = std::make_shared<FDSet> (plis.size());
    }


    std::shared_ptr<nonFDList> getNonFDCandidate(const std::vector<std::pair<size_t , size_t>> &comparisonSuggestions);

    void match(boost::dynamic_bitset<> &bSet, int lhs, int rhs);

};

void runWindow(Sampler &sampler, Efficiency &efficiency,
                       std::shared_ptr<util::PositionListIndex> pli, std::shared_ptr<nonFDList> nonFds );

}


