/**
 * @file FixedBinAxis.h
 * @author Jon Burr
 * @brief Axis with a fixed bin width or number of bins
 * @version 0.0.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5HISTOGRAMS_FIXEDBINAXIS_H
#define H5HISTOGRAMS_FIXEDBINAXIS_H

#include "H5Histograms/NumericAxis.h"
#include "H5Composites/TypeRegister.h"
#include "H5Composites/CompositeDefinition.h"
#include "H5Composites/DTypes.h"

namespace H5Histograms
{
    class FixedBinAxis : public IAxisFactory::Registree<FixedBinAxis>, public NumericAxis
    {
    public:
        /// Describes how the axis can be extended
        enum class ExtensionType : char
        {
            NoExtension,     ///< Axis cannot be extended
            PreserveNBins,   ///< Extend the axis such that the number of bins is preserved
            PreserveBinWidth ///< Extend the axis such that the bin width is preserved
        };

        friend class H5Composites::CompositeDefinition<FixedBinAxis>;
        static const H5Composites::CompositeDefinition<FixedBinAxis> &compositeDefinition();

        FixedBinAxis(
            const std::string &label,
            std::size_t nBins,
            double min,
            double max,
            ExtensionType extension = ExtensionType::NoExtension);
        FixedBinAxis(const void *buffer, const H5::DataType &dtype);

        H5::DataType h5DType() const override;
        void writeBuffer(void *buffer) const override;

        static std::string registeredName() { return "H5Histograms::FixedBinAxis"; }

        /// If the axis is extendable
        bool isExtendable() const override { return m_extension != ExtensionType::NoExtension; }

        /// The number of non-overflow bins on the axis
        std::size_t nBins() const override { return m_nBins; }

        /// The number of bins on the axis (including under/overflow)
        std::size_t fullNBins() const override;

        /// Get the index of a bin from its value
        IAxis::index_t findBin(const IAxis::value_t &value) const override;

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

        /// Get the width of a single bin
        double binWidth() const;

    private:
        std::size_t m_nBins;
        double m_min;
        double m_max;
        ExtensionType m_extension;
    }; //> end class FixedBinAxis
};     //> end namespace H5Histograms

H5COMPOSITES_DECLARE_STATIC_H5DTYPE(H5Histograms::FixedBinAxis::ExtensionType);
#endif //> !H5HISTOGRAMS_FIXEDBINAXIS_H