#include "H5Histograms/Histogram.h"
#include "H5Composites/FixedLengthVectorTraits.h"

namespace {
    template <typename STORAGE>
    std::pair<STORAGE, STORAGE> mkPair(const STORAGE &first, const STORAGE &second)
    {
        return std::make_pair(first, second);
    }

    template <typename STORAGE>
    std::pair<std::reference_wrapper<STORAGE>, std::reference_wrapper<STORAGE>> mkPair(STORAGE &first, STORAGE &second)
    {
        return std::make_pair(std::ref(first), std::ref(second));
    }
}

namespace H5Histograms
{
    template <typename STORAGE>
    const H5Composites::CompositeDefinition<Histogram<STORAGE>> &Histogram<STORAGE>::compositeDefinition()
    {
        static H5Composites::CompositeDefinition<Histogram> definition;
        static bool init = false;
        if (!init)
        {
            definition.template add<H5Composites::FLVector<IAxisUPtr>>(&Histogram::m_axes, "axes");
            definition.template add(&Histogram::m_nEntries, "nEntries");
            definition.template add<H5Composites::FLVector<STORAGE>>(&Histogram::m_counts, "counts");
            definition.template add<H5Composites::FLVector<STORAGE>>(&Histogram::m_sumW2, "sumW2");
            init = true;
        }
        return definition;
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST>::Iterator(histogram_t &histogram)
        : m_idxItr(histogram.m_indexer.begin()),
          m_histo(histogram),
          m_offset(0),
          m_value(std::tie(histogram.m_counts.front(), histogram.m_sumW2.front()))
    {
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST>::Iterator(histogram_t &histogram, const Histogram::value_t &values)
        : m_idxItr(histogram.m_indexer.axisSizes(), histogram.axisOffsetsFromValues(values)),
          m_histo(histogram),
          m_offset(m_idxItr.offset()),
          m_value(std::tie(histogram.m_counts.at(m_offset), histogram.m_sumW2.at(m_offset)))
    {
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST> Histogram<STORAGE>::Iterator<CONST>::createEnd(histogram_t &histogram)
    {
        // Bit of a cludge - create an iterator to the start and then modify it to be at the end
        Iterator itr(histogram);
        itr.m_idxItr = histogram.m_indexer.end();
        itr.m_offset = itr.m_idxItr.offset();
        itr.m_value.reset();
        return itr;
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST>::Iterator(const Iterator<false> &other)
        : m_idxItr(other.m_idxItr),
          m_histo(other.m_histo),
          m_offset(other.m_offset),
          m_value(other.m_value)
    {}

    template <typename STORAGE>
    template <bool CONST>
    HistogramBase::index_t Histogram<STORAGE>::Iterator<CONST>::indices() const
    {
        std::vector<std::size_t> offsets = *m_idxItr;
        HistogramBase::index_t ret(offsets.size());
        for (std::size_t idx = 0; idx < offsets.size(); ++idx)
            ret[idx] = m_histo.axis(idx).indexFromBinOffset(offsets[idx]);
        return ret;
    }

    template <typename STORAGE>
    template <bool CONST>
    bool Histogram<STORAGE>::Iterator<CONST>::operator==(const Iterator &other) const
    {
        return &m_histo == &other.m_histo && m_offset == other.m_offset;
    }

    template <typename STORAGE>
    template <bool CONST>
    bool Histogram<STORAGE>::Iterator<CONST>::operator!=(const Iterator &other) const
    {
        return !(*this == other);
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST> &Histogram<STORAGE>::Iterator<CONST>::operator++()
    {
        ++m_idxItr;
        ++m_offset;
        if (m_offset == m_histo.m_counts.size())
            // exhausted
            m_value.reset();
        else
            m_value = std::tie(m_histo.m_counts[m_offset], m_histo.m_sumW2[m_offset]);
        return *this;
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST> Histogram<STORAGE>::Iterator<CONST>::operator++(int)
    {
        Iterator itr = *this;
        ++*this;
        return itr;
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST> &Histogram<STORAGE>::Iterator<CONST>::operator--()
    {
        // Will throw an exception if we're going past the beginning
        --m_idxItr;
        --m_offset;
        m_value = std::tie(m_histo.m_counts[m_offset], m_histo.m_sumW2[m_offset]);
        return *this;
    }

    template <typename STORAGE>
    template <bool CONST>
    Histogram<STORAGE>::Iterator<CONST> Histogram<STORAGE>::Iterator<CONST>::operator--(int)
    {
        Iterator itr = *this;
        --*this;
        return itr;
    }

    template <typename STORAGE>
    Histogram<STORAGE>::Histogram(const void *buffer, const H5::DataType &dtype) : HistogramBase({})
    {
        compositeDefinition().readBuffer(*this, buffer, dtype);
        calculateStrides();
    }

    template <typename STORAGE>
    Histogram<STORAGE>::Histogram(std::vector<std::unique_ptr<IAxis>> &&axes)
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
    void Histogram<STORAGE>::writeBuffer(void *buffer) const
    {
        compositeDefinition().writeBuffer(*this, buffer);
    }

    template <typename STORAGE>
    void Histogram<STORAGE>::fill(const value_t &values, STORAGE weight)
    {
        std::size_t offset = binOffsetFromValues(values);
        if (offset == SIZE_MAX)
        {
            std::vector<IAxis::ExtensionInfo> extensions = extendAxes(values, offset);
            resize(extensions);
        }
        m_counts.at(offset) += weight;
        m_sumW2.at(offset) += weight * weight;
        ++m_nEntries;
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
    void Histogram<STORAGE>::resize(const std::vector<IAxis::ExtensionInfo> &extensions)
    {
        if (extensions.size() != nDims())
            throw std::invalid_argument("Number of axis extensions does not match the number of dimensions!");
        std::vector<STORAGE> oldCounts = std::move(m_counts);
        std::vector<STORAGE> oldSumW2 = std::move(m_sumW2);
        // Right now the actual axes are updated but the indexer is not
        ArrayIndexer oldIndexer = m_indexer;
        // Now update the indexer
        calculateStrides();
        std::size_t n = fullNBins();
        // Make enough space for the new counts
        m_counts.assign(n, 0);
        m_sumW2.assign(n, 0);
        // Need to iterate over every original bin
        for (const std::vector<std::size_t> &oldOffsets : oldIndexer)
        {
            std::size_t oldOffset = oldIndexer.offset_noCheck(oldOffsets);
            std::vector<std::size_t> newOffsets(nDims());
            for (std::size_t idx = 0; idx < nDims(); ++idx)
                newOffsets[idx] = extensions[idx].func(oldOffsets[idx]);
            std::size_t newOffset = m_indexer.offset_noCheck(newOffsets);
        
            m_counts.at(newOffset) += oldCounts[oldOffset];
            m_sumW2.at(newOffset) += oldSumW2[oldOffset];
        }
    }

    template <typename STORAGE>
    Histogram<STORAGE> &Histogram<STORAGE>::operator+=(const Histogram &h)
    {
        if (nDims() != h.nDims())
            throw std::invalid_argument("Dimensions do not match");
        std::vector<IAxis::ExtensionInfo> extensions;
        extensions.reserve(nDims());
        for (std::size_t idx = 0; idx < nDims(); ++idx)
            extensions.push_back(axis(idx).compareAxis(h.axis(idx)));
        // Now iterate over the bins in the old histogram
        for (const std::vector<std::size_t> &oldOffsets : h.m_indexer)
        {
            std::size_t oldOffset = h.m_indexer.offset_noCheck(oldOffsets);
            std::vector<std::size_t> newOffsets(nDims());
            for (std::size_t idx = 0; idx < nDims(); ++idx)
                newOffsets[idx] = extensions[idx].func(oldOffsets[idx]);
            std::size_t newOffset = m_indexer.offset_noCheck(newOffsets);
        
            m_counts.at(newOffset) += h.m_counts[oldOffset];
            m_sumW2.at(newOffset) += h.m_sumW2[oldOffset];
        }
        return *this;
    }

    // Force the instantiation of the types we defined before
    template class Histogram<int>;
    template class Histogram<int>::Iterator<true>;
    template class Histogram<int>::Iterator<false>;
    template class Histogram<unsigned int>;
    template class Histogram<unsigned int>::Iterator<true>;
    template class Histogram<unsigned int>::Iterator<false>;
    template class Histogram<char>;
    template class Histogram<char>::Iterator<true>;
    template class Histogram<char>::Iterator<false>;
    template class Histogram<signed char>;
    template class Histogram<signed char>::Iterator<true>;
    template class Histogram<signed char>::Iterator<false>;
    template class Histogram<unsigned char>;
    template class Histogram<unsigned char>::Iterator<true>;
    template class Histogram<unsigned char>::Iterator<false>;
    template class Histogram<short>;
    template class Histogram<short>::Iterator<true>;
    template class Histogram<short>::Iterator<false>;
    template class Histogram<unsigned short>;
    template class Histogram<unsigned short>::Iterator<true>;
    template class Histogram<unsigned short>::Iterator<false>;
    template class Histogram<long>;
    template class Histogram<long>::Iterator<true>;
    template class Histogram<long>::Iterator<false>;
    template class Histogram<long long>;
    template class Histogram<long long>::Iterator<true>;
    template class Histogram<long long>::Iterator<false>;
    template class Histogram<unsigned long>;
    template class Histogram<unsigned long>::Iterator<true>;
    template class Histogram<unsigned long>::Iterator<false>;
    template class Histogram<unsigned long long>;
    template class Histogram<unsigned long long>::Iterator<true>;
    template class Histogram<unsigned long long>::Iterator<false>;
    template class Histogram<float>;
    template class Histogram<float>::Iterator<true>;
    template class Histogram<float>::Iterator<false>;
    template class Histogram<double>;
    template class Histogram<double>::Iterator<true>;
    template class Histogram<double>::Iterator<false>;

} //> end namespace H5Histograms
