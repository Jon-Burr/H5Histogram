#include "H5Histograms/HistogramBase.h"
#include "H5Composites/CompDTypeUtils.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/MergeUtils.h"
#include "H5Histograms/Histogram.h"
#include "H5Composites/DTypeDispatch.h"

#include <tuple>

namespace {
    // Helper struct to break down the data of a histogram
    struct HistogramData
    {
        HistogramData(const H5::CompType &dtype, const void *buffer)
        {
            H5::CompType axesType = dtype.getMemberCompType(dtype.getMemberIndex("axes"));
            const void *axesData = H5Composites::getMemberPointer(buffer, dtype, "axes");
            axes.reserve(axesType.getNmembers());
            for (std::size_t idx = 0; idx < axesType.getNmembers(); ++idx)
            {
                const void *axisData = H5Composites::getMemberPointer(axesData, axesType, idx);
                H5::CompType axisType = axesType.getMemberCompType(idx);
                axes.emplace_back(
                    H5Composites::readCompositeElement<H5Composites::TypeRegister::id_t>(axisData, axisType, "typeID"),
                    axisType.getMemberCompType(axisType.getMemberIndex("data")),
                    H5Composites::getMemberPointer(axisData, axisType, "data")
                );
            }
            countsDType = dtype.getMemberDataType(dtype.getMemberIndex("counts")).getSuper();
            sumW2DType = dtype.getMemberDataType(dtype.getMemberIndex("sumW2")).getSuper();
        }
        std::vector<std::tuple<H5Composites::TypeRegister::id_t, H5::DataType, const void*>> axes;
        H5::DataType countsDType;
        H5::DataType sumW2DType;
    };

    template <typename T>
    struct HistogramBuilder
    {
        H5Composites::H5Buffer operator()(
            std::vector<std::unique_ptr<H5Histograms::IAxis>> &&axes,
            const std::vector<std::pair<H5::DataType, const void*>> &buffers)
        {
            H5Histograms::Histogram<T> h(std::move(axes));
            for (const std::pair<H5::DataType, const void *> &buffer : buffers)
                h += H5Composites::fromBuffer<H5Histograms::Histogram<T>>(buffer.second, buffer.first);
            return H5Composites::toBuffer(h);
        }
    };
}

namespace H5Histograms
{

    HistogramBase::HistogramBase(std::vector<std::unique_ptr<IAxis>> &&axes)
        : m_axes(std::move(axes)), m_indexer({})
    {
        calculateStrides();
    }

    H5Composites::H5Buffer HistogramBase::mergeBuffers(const std::vector<std::pair<H5::DataType, const void *>> &buffers)
    {
        std::optional<std::size_t> nDims;
        std::vector<std::optional<H5Composites::TypeRegister::id_t>> axisTypeIDs;
        std::vector<std::vector<std::pair<H5::DataType, const void *>>> axisData;
        std::vector<H5::DataType> countDTypes;
        for (const std::pair<H5::DataType, const void *> &buffer : buffers)
        {
            HistogramData data(buffer.first.getId(), buffer.second);
            if (!nDims.has_value())
            {
                axisTypeIDs.resize(data.axes.size());
                axisData.resize(data.axes.size());
            }
            if (!H5Composites::enforceEqual(nDims, data.axes.size()))
                throw std::invalid_argument("Dimensions do not match!");
            for (std::size_t idx = 0; idx < data.axes.size(); ++idx)
            {
                if (!H5Composites::enforceEqual(axisTypeIDs.at(idx), std::get<0>(data.axes.at(idx))))
                    throw std::invalid_argument("Axis types do not match!");
                axisData.at(idx).emplace_back(std::get<1>(data.axes.at(idx)), std::get<2>(data.axes.at(idx)));
            }
            countDTypes.push_back(data.countsDType);
            countDTypes.push_back(data.sumW2DType);
        }
        // Now get the merged axes
        std::vector<std::unique_ptr<IAxis>> axes;
        axes.reserve(axisTypeIDs.size());
        for (std::size_t idx = 0; idx < axisTypeIDs.size(); ++idx)
        {
            H5Composites::H5Buffer merged = H5Composites::MergeFactory::instance().merge(*axisTypeIDs.at(idx), axisData.at(idx));
            axes.push_back(IAxisFactory::instance().create(*axisTypeIDs.at(idx), merged));
        }
        // Get a common data type
        H5::PredType common = H5Composites::getCommonNumericDType(countDTypes);
        return H5Composites::apply_if<std::is_arithmetic, HistogramBuilder>(common, std::move(axes), buffers);
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

    std::vector<std::size_t> HistogramBase::axisOffsetsFromValues(const value_t &values) const
    {
        if (nDims() != values.size())
            throw std::invalid_argument("Incorrect number of values provided");
        std::vector<std::size_t> offsets(nDims());
        for (std::size_t idx = 0; idx < nDims(); ++idx)
            offsets[idx] = axis(idx).binOffsetFromValue(values.at(idx));
        return offsets;
    }

    std::vector<std::size_t> HistogramBase::axisOffsetsFromIndices(const index_t &indices) const
    {
        if (nDims() != indices.size())
            throw std::invalid_argument("Incorrect number of indices provided");
        std::vector<std::size_t> offsets(nDims());
        for (std::size_t idx = 0; idx < nDims(); ++idx)
            offsets[idx] = axis(idx).binOffsetFromIndex(indices.at(idx));
        return offsets;
    }

    std::size_t HistogramBase::binOffsetFromValues(const value_t &values) const
    {
        return m_indexer.offset_noCheck(axisOffsetsFromValues(values));
    }

    std::size_t HistogramBase::binOffsetFromIndices(const index_t &indices) const
    {
        return m_indexer.offset_noCheck(axisOffsetsFromIndices(indices));
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
        return m_indexer.nEntries();
    }

    void HistogramBase::calculateStrides()
    {
        std::vector<std::size_t> sizes(nDims(), 0);
        for (std::size_t idx = 0; idx < nDims(); ++idx)
            sizes[idx] = axis(idx).fullNBins();
        m_indexer = sizes;
    }

    std::vector<IAxis::ExtensionInfo> HistogramBase::extendAxes(const value_t &values, std::size_t &offset)
    {
        std::vector<IAxis::ExtensionInfo> ret;
        ret.reserve(nDims());
        std::vector<std::size_t> offsets(nDims(), 0);
        for (std::size_t idx = 0; idx < nDims(); ++idx)
        {
            std::size_t axisOffset = 0;
            ret.push_back(m_axes.at(idx)->extendAxis(values.at(idx), axisOffset));
            offsets.push_back(axisOffset);
        }
        offset = m_indexer.offset_noCheck(offsets);
        return ret;
    }
}