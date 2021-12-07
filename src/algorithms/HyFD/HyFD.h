#pragma once 

#include "RelationData.h"
#include "CSVParser.h"
#include "PositionListIndex.h"
#include <string>


#include <chrono>
#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <filesystem>

#include "ColumnCombination.h"
#include "ColumnData.h"
#include "ColumnLayoutRelationData.h"
#include "PliBasedFDAlgorithm.h"
#include "RelationalSchema.h"
#include "LatticeLevel.h"
#include "LatticeVertex.h"
#include "Sampler.h"
#include "Inductor.h"
#include "Validator.h"

namespace HyFD {

    using PLIS = std::vector<std::shared_ptr<util::PositionListIndex>>;

    class HyFD : public PliBasedFDAlgorithm {
    private:
        unsigned long long executeInternal() override;

    public:
        explicit HyFD(std::filesystem::path const &path,
                      char separator = ',', bool hasHeader = true,
                      bool const is_null_equal_null = true,
                      std::vector<std::string_view> phase_names = {"FD mining"}) :
                PliBasedFDAlgorithm(path, separator, hasHeader, is_null_equal_null, std::move(phase_names)) {}
    };

}
