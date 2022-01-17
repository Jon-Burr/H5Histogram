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
#include "H5Composites/MergeFactory.h"

namespace H5Histograms
{
    class FixedBinAxis : public NumericAxis
    {
    public:
        H5HISTOGRAMS_DECLARE_IAXIS()
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
        
        void merge(const FixedBinAxis &other);

        static std::string registeredName() { return "H5Histograms::FixedBinAxis"; }

        /// If the axis is extendable
        bool isExtendable() const override { return m_extension != ExtensionType::NoExtension; }

        /// The number of non-overflow bins on the axis
        std::size_t nBins() const override { return m_nBins; }

        /// The number of bins on the axis (including under/overflow)
        std::size_t fullNBins() const override;

        double min() const { return m_min; }
        double max() const { return m_max; }

        /// Get the index of a bin from its value
        IAxis::index_t findBin(const IAxis::value_t &value) const override;

        /**
         * @brief Extend the axis to contain a particular value
         * 
         * @param value The value to contain
         * @param[out] offset The offset of the bin containing the specified value
         */
        ExtensionInfo extendAxis(const IAxis::value_t &value, std::size_t &offset) override;

        ExtensionInfo compareAxis(const IAxis &other) const override;

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