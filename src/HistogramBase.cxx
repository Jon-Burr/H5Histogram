#include "H5Histograms/HistogramBase.h"

namespace H5Histograms
{

    HistogramBase::HistogramBase(std::vector<std::unique_ptr<IAxis>> &&axes)
        : m_axes(std::move(axes))
    {
        calculateStrides();
    }

    std::size_t HistogramBase::nDims() const
    {
        return m_axes.size();
    }

    const IAxis &HistogramBase::axis(std::size_t idx) const
    {
        return *m_axes.at(idx);
    }

    std::vector<IAxis::index_t> HistogramBase::findBin(const value_t &values) const
    {
        if (nDims() != values.size())
            throw std::invalid_argument("Incorrect number of values provided");
        std::vector<IAxis::index_t> indices;
        indices.reserve(nDims());
        for (std::size_t idx = 0; idx < nDims(); ++idx)
            indices.push_back(axis(idx).findBin(values.at(idx)));
        return indices;
    }

    std::size_t HistogramBase::binOffsetFromValues(const value_t &values) const
    {
        if (nDims() != values.size())
            throw std::invalid_argument("Incorrect number of values provided");
        std::size_t offset = 0;
        for (std::size_t idx = 0; idx < nBins(); ++idx)
        {
            std::size_t axisOffset = axis(idx).binOffsetFromValue(values.at(idx));
            if (axisOffset == SIZE_MAX)
                return SIZE_MAX;
            offset += m_strides.at(idx) * axisOffset;
        }
        return offset;
    }

    std::size_t HistogramBase::binOffsetFromIndices(const index_t &indices) const
    {
        if (nDims() != indices.size())
            throw std::invalid_argument("Incorrect number of indices provided");
        std::size_t offset = 0;
        for (std::size_t idx = 0; idx < nBins(); ++idx)
        {
            std::size_t axisOffset = axis(idx).binOffsetFromIndex(indices.at(idx));
            if (axisOffset == SIZE_MAX)
                return SIZE_MAX;
            offset += m_strides.at(idx) * axisOffset;
        }
        return offset;
    }

    bool HistogramBase::contains(const value_t &values) const
    {
        return binOffsetFromValues(values) != SIZE_MAX;
    }

    std::size_t HistogramBase::nBins() const
    {
        std::size_t n = 1;
        for (std::size_t idx = 0; idx < nDims(); ++idx)
            n *= axis(idx).nBins();
        return n;
    }

    std::size_t HistogramBase::fullNBins() const
    {
        return m_strides.back();
    }

    void HistogramBase::calculateStrides()
    {
        m_strides.assign(nDims() + 1, 0);
        std::size_t stride = 1;
        for (std::size_t idx = nDims() - 1; idx >= 0; --idx)
        {
            m_strides.at(idx) = stride;
            stride *= axis(idx).fullNBins();
        }
        m_strides[nDims() + 1] = stride;
    }

    std::vector<std::vector<std::vector<std::size_t>>> extendAxes(const value_t &values, std::size_t &offset)
    {
        std::vector<std::vector<std::vector<std::size_t>>> ret;
        ret.reserve(nDims());
        offset = 0;
        for (std::size_t idx = 0; idx < nDims(); ++idx)
        {
            std::size_t thisOffset = 0;
            ret.push_back(axis(idx).extendAxis(values.at(idx), thisOffset));
            offset += thisOffset * m_strides.at(idx);
        }
        return ret;
    }
}