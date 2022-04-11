#include <gmock/gmock.h>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "BuiltIn.h"
#include "CSVParser.h"
#include "ColumnLayoutTypedRelationData.h"
#include "CsvStats.h"
#include "NumericType.h"

namespace tests {

struct CsvStatsParams {
    std::string_view dataset_;
    std::vector<std::optional<std::byte*>> sum_;
    char const sep_;
    bool const has_header_;

    CsvStatsParams(std::string_view dataset, std::vector<std::optional<std::byte*>> sum,
                   char const sep = ',',
                   bool const has_header = true) noexcept
        : dataset_(dataset), sum_(std::move(sum)), sep_(sep), has_header_(has_header)  {}
};

class TestCsvStats : public ::testing::TestWithParam<CsvStatsParams> {};

TEST_P(TestCsvStats, TestSum) {
    CsvStatsParams const& p = GetParam();
    CsvStats stats(p.dataset_, p.sep_, p.has_header_);
    int i = 0;
    for (auto sum : p.sum_) {
        ASSERT_EQ(sum, stats.GetSum(i++));
    }
}

TEST(TestCsvStats, TestCsvSum) {
    CsvStats stats("BernoulliRelation.csv", ',', true);
    mo::TypedColumnData const& col = stats.col_data_[1];
    mo::INumericType const& type = static_cast<mo::INumericType const&>(col.GetType());
    auto s = type.GetValue<mo::Int>(stats.GetSum(1));
    ASSERT_EQ(s, 3);
}

TEST(TestCsvStats, TestCsvAvg) {
    CsvStats stats("BernoulliRelation.csv", ',', true);
    mo::TypedColumnData const& col = stats.col_data_[1];
    mo::INumericType const& type = static_cast<mo::INumericType const&>(col.GetType());
    auto s = type.GetValue<mo::Double>(stats.GetAvg(1));
    ASSERT_EQ(s, 0.5);
}

TEST(TestCsvStats, TestCsvSTD) {
    CsvStats stats("BernoulliRelation.csv", ',', true);
    mo::TypedColumnData const& col = stats.col_data_[1];
    mo::INumericType const& type = static_cast<mo::INumericType const&>(col.GetType());
    auto s = type.GetValue<mo::Double>(stats.GetSTD(1));
    mo::Double expected = 0.547722557505166113456969782801;
    EXPECT_DOUBLE_EQ(s, expected);
}

TEST(TestCsvStats, TestCsvSkewness) {
    CsvStats stats("BernoulliRelation.csv", ',', true);
    mo::TypedColumnData const& col = stats.col_data_[1];
    mo::INumericType const& type = static_cast<mo::INumericType const&>(col.GetType());
    auto s = type.GetValue<mo::Double>(stats.GetSkewness(1));
    mo::Double expected = 0.0;
    EXPECT_DOUBLE_EQ(s, expected);
}

TEST(TestCsvStats, TestCount) {
    CsvStats stats("CIPublicHighway10k.csv", ',', true);
    //mo::TypedColumnData const& col = stats.col_data_[1];
    //mo::INumericType const& type = static_cast<mo::INumericType const&>(col.GetType());
    auto s = stats.Count(3);
    ASSERT_EQ(s, 3);
}
/*INSTANTIATE_TEST_SUITE_P(
    TypeSystem, TestCsvStats,
    ::testing::Values(
        CsvStatsParams("BernoulliRelation.csv", {static_cast<std::byte*>(1), mo::Int(3), mo::Int(3), mo::Int(3), mo::Int(5), mo::Int(5)})));*/

};  // namespace tests
