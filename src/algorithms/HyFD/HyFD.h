#pragma once 

#include <string>
#include <vector>
#include <filesystem>

#include "PositionListIndex.h"
#include "PliBasedFDAlgorithm.h"

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
