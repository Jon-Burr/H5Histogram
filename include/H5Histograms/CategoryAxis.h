/**
 * @file CategoryAxis.h
 * @author Jon Burr 
 * @brief Axis implementation that bins based on string categories
 * @version 0.0.0
 * @date 2022-01-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5HISTOGRAMS_CATEGORYAXIS_H
#define H5HISTOGRAMS_CATEGORYAXIS_H

#include "H5Histograms/IAxis.h"
#include "H5Composites/CompositeDefinition.h"
#include "H5Composites/TypeRegister.h"
#include <string>
#include <vector>

namespace H5Histograms
{
    class CategoryAxis : public IAxisFactory::Registree<CategoryAxis>, public IAxis
    {
        friend class H5Composites::CompositeDefinition<CategoryAxis>;
        static const H5Composites::CompositeDefinition<CategoryAxis> &compositeDefinition();

    public:
        using value_t = std::string;
        using index_t = std::string;

        CategoryAxis(const void *buffer, const H5::DataType &dtype);
        CategoryAxis(const std::string &label, const std::vector<std::string> &categories, bool extendable = false);

        void writeBuffer(void *buffer) const override;
        H5::DataType h5DType() const override;

        static index_t overflowName() { return "UNCATEGORISED"; }

        static std::string registeredName() { return "H5Histograms::CategoryAxis"; }
        /// The type of this axis
        Type axisType() const override { return Type::Category; }

        /// The axis label
        std::string label() const override { return m_label; }

        /// If the axis is extendable
        bool isExtendable() const override { return m_extendable; }

        /// The number of non-overflow bins on the axis
        std::size_t nBins() const override { return m_categories.size(); }

        /// The number of bins on the axis (including under/overflow)
        std::size_t fullNBins() const override;

        /// Get the offset of a bin from its value
        std::size_t binOffsetFromValue(const IAxis::value_t &value) const override;

        /// Get the offset of a bin from its index
        std::size_t binOffsetFromIndex(const IAxis::index_t &index) const override;

        /// Get the index of a bin from its value. If it's the overflow bin return 'UNCATEGORISED'
        IAxis::index_t findBin(const IAxis::value_t &value) const override;

        /// Whether the axis contains a bin that holds the given value
        bool containsValue(const IAxis::value_t &value) const override;

        /**
         * @brief Extend the axis to contain a particular value
         * 
         * @param value The value to contain
         * @param[out] offset The offset of the bin containing the specified value
         * @return A mapping of new bin offsets to the old bin offsets
         * 
         * The return value is a vector with one entry per bin in the new array. Each value in this
         * vector is a list of bin numbers in the old array. If this list is empty, the new bin has
         * 0 entries, if it has exactly 1 entry then it copies the entries in that bin. Otherwise 
         * its contents is the sum of the entries in the old bins.
         * 
         * If the axis doesn't need to be extended to accommodate the value the returned vector is empty.
         * This will always be the case if the axis is not extendable.
         */
        std::vector<std::vector<std::size_t>> extendAxis(const IAxis::value_t &value, std::size_t &offset) override;

    private:
        std::string m_label;
        std::vector<std::string> m_categories;
        bool m_extendable;
    }; //> end class CategoryAxis
}

#endif //< !H5HISTOGRAMS_CATEGORYAXIS_H