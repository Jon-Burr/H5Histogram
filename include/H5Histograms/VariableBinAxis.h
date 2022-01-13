/**
 * @file VariableBinAxis.h
 * @author Jon Burr
 * @brief Axis class for bins of varying width
 * @version 0.0.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5HISTOGRAMS_VARIABLEBINAXIS_H
#define H5HISTOGRAMS_VARIABLEBINAXIS_H

#include "H5Histograms/NumericAxis.h"
#include "H5Composites/TypeRegister.h"
#include "H5Composites/CompositeDefinition.h"
#include "H5Composites/MergeFactory.h"

namespace H5Histograms
{
    class VariableBinAxis : 
        public IAxisFactory::Registree<VariableBinAxis>,
        public H5Composites::MergeFactory::Registree<VariableBinAxis>,
        public NumericAxis
    {
    public:
        friend class H5Composites::CompositeDefinition<VariableBinAxis>;
        static const H5Composites::CompositeDefinition<VariableBinAxis> &compositeDefinition();

        VariableBinAxis(const std::string &label, const std::vector<double> &edges);
        VariableBinAxis(const void *buffer, const H5::DataType &dtype);

        H5::DataType h5DType() const override;
        void writeBuffer(void *buffer) const override;
        static H5Composites::H5Buffer mergeBuffers(const std::vector<std::pair<H5::DataType, const void*>> &buffers);

        static std::string registeredName() { return "H5Histograms::VariableBinAxis"; }

        /// If the axis is extendable
        bool isExtendable() const override { return false; }

        /// The number of non-overflow bins on the axis
        std::size_t nBins() const override;

        /// The number of bins on the axis (including under/overflow)
        std::size_t fullNBins() const override;

        /// Get the index of a bin from its value
        IAxis::index_t findBin(const IAxis::value_t &value) const override;

        /**
         * @brief Extend the axis to contain a particular value
         * 
         * @param value The value to contain
         * @param[out] offset The offset of the bin containing the specified value
         * 
         * Given that variable bin axes are not (currently) extendable, this always returns an
         * empty object
         */
        ExtensionInfo extendAxis(const IAxis::value_t &value, std::size_t &offset) override;

        ExtensionInfo compareAxis(const IAxis &other) const;
    private:
        std::vector<double> m_edges;
    }; //> end class VariableBinAxis
};     //> end namespace H5Histograms

#endif //> !H5HISTOGRAMS_VARIABLEBINAXIS_H