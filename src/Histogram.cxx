#include "H5Histograms/Histogram.h"
#include "H5Composites/FixedLengthVectorTraits.h"

namespace H5Histograms
{
    template <typename STORAGE>
    const H5Composites::CompositeDefinition<Histogram<STORAGE>> &Histogram<STORAGE>::compositeDefinition()
    {
        static H5Composites::CompositeDefinition<Histogram> &definition;
        static bool init = false;
        if (!init)
        {
            definition.add<H5Composites::FLVector<IAxisFactor::UPtr>>(&Histogram::m_axes, "axes");
            definition.add(&Histogram::m_nEntries, "nEntries");
            definition.add<H5Composites::FLVector<STORAGE>>(&Histogram::m_counts, "counts");
            definition.add<H5Composites::FLVector<STORAGE>>(&Histogram::m_sumW2, "sumW2");
            init = true;
        }
        return definition;
    }

    template <typename STORAGE>
    Histogram<STORAGE>::Histogram(const void *buffer, const H5::DataType &dtype) : HistogramBase({})
    {
        compositeDefinition().readBuffer(*this, buffer, dtype);
        calculateStrides();
    }

    template <typename STORAGE>
    Histogram<STORAGE>::Histogram(const std::vector<std::unique_ptr<IAxis>> &&axes)
        : HistogramBase(std::move(axes)),
          m_nEntries(0),
          m_counts(fullNBins(), 0),
          m_sumW2(fullNBins(), 0)
    {
    }

    template <typename STORAGE>
    H5::DataType Histogram<STORAGE>::h5DType() const
    {
        return compositeDefinition().dtype(*this);
    }

    template <typename STORAGE>
    void Histogram<STORAGE>::write(void *buffer) const
    {
        compositeDefinition().writeBuffer(*this, buffer);
    }

    template <typename STORAGE>
    void Histogram<STORAGE>::fill(const value_t &values, STORAGE weight)
    {
        std::size_t offset = binOffsetFromValues(values);
        if (offset == SIZE_MAX)
        {
            std::vector<std::vector<std::vector<std::size_t>>> extensions = extendAxes(values, offset);
            resize(extensions);
        }
        m_counts.at(offset) += weight;
        m_sumW2.at(offset) += weight * weight;
    }

    template <typename STORAGE>
    STORAGE &Histogram<STORAGE>::contents(const index_t &indices)
    {
        return m_counts.at(binOffsetFromIndices(indices));
    }

    template <typename STORAGE>
    STORAGE Histogram<STORAGE>::contents(const index_t &indices) const
    {
        return m_counts.at(binOffsetFromIndices(indices));
    }

    template <typename STORAGE>
    STORAGE &Histogram<STORAGE>::sumW2(const index_t &indices)
    {
        return m_sumW2.at(binOffsetFromIndices(indices));
    }

    template <typename STORAGE>
    STORAGE Histogram<STORAGE>::sumW2(const index_t &indices) const
    {
        return m_sumW2.at(binOffsetFromIndices(indices));
    }

    template <typename STORAGE>
    void Histogram<STORAGE>::resize(const std::vector<std::vector<std::vector<std::size_t>>> &extensions)
    {
        if (extensions.size() != nDims())
            throw std::invalid_argument("Number of axis extensions does not match the number of dimensions!");

        std::vector<STORAGE> oldCounts = std::move(m_counts);
        std::vector<STORAGE> oldSumW2 = std::move(m_sumW2);
        // Right now the actual axes are updated but the strides are not.
        std::vector<std::size_t> oldStrides = m_strides;
        // Now update the strides
        calculateStrides();
        std::size_t n = fullNBins();
        // Make enough space for the new counts
        m_counts.assign(n);
        m_sumW2.assign(n);
    }

} //> end namespace H5Histograms