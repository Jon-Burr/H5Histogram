/**
 * @file HistogramBase.h
 * @author Jon Burr
 * @brief ND Histogram implementation
 * @version 0.0.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5HISTOGRAMS_HISTOGRAMBASE_H
#define H5HISTOGRAMS_HISTOGRAMBASE_H

#include "H5Composites/IBufferWriter.h"
#include "H5Histograms/ArrayIndexer.h"
#include "H5Histograms/IAxis.h"
#include "H5Composites/MergeFactory.h"

#include <vector>
#include <memory>

namespace H5Histograms
{
    class HistogramBase : public H5Composites::IBufferWriter
    {
    public:
        H5COMPOSITES_DECLARE_TYPEID()
        H5COMPOSITES_DECLARE_MERGE()
        using value_t = std::vector<IAxis::value_t>;
        using index_t = std::vector<IAxis::index_t>;
        HistogramBase(std::vector<std::unique_ptr<IAxis>> &&axes);

        static std::string registeredName() { return "H5Histograms::Histogram"; }

        std::size_t nDims() const;

        const IAxis &axis(std::size_t idx) const;

        std::vector<IAxis::index_t> findBin(const value_t &values) const;

        std::vector<std::size_t> axisOffsetsFromValues(const value_t &values) const;

        std::vector<std::size_t> axisOffsetsFromIndices(const index_t &indices) const;

        std::size_t binOffsetFromValues(const value_t &values) const;

        std::size_t binOffsetFromIndices(const index_t &values) const;

        bool contains(const value_t &values) const;

        std::size_t nBins() const;

        std::size_t fullNBins() const;

    protected:
        void calculateStrides();

        std::vector<IAxis::ExtensionInfo> extendAxes(const value_t &values, std::size_t &offset);

        std::vector<std::unique_ptr<IAxis>> m_axes;
        ArrayIndexer m_indexer;
    }; //> end class HistogramBase
} //> end namespace H5Histograms

#endif //> !H5HISTOGRAMS_HISTOGRAM_H