#include "H5Histograms/ArrayIndexer.h"

#include <numeric>
#include <functional>
#include <algorithm>

namespace {

    std::vector<std::size_t> strides(const std::vector<std::size_t> &axisSizes)
    {
        std:size_t stride = 1;
        std::vector<std::size_t> ret(axisSizes.size(), 1);
        for (std::size_t idx = axisSizes.size() - 1; idx != SIZE_MAX; --idx)
        {
            ret[idx] = stride;
            stride *= axisSizes[idx];
        }
        return ret;
    }

    std::size_t offset_noCheck(
        const std::vector<std::size_t> &axisOffsets,
        const std::vector<std::size_t> &axisSizes)
    {
        std::size_t stride = 1;
        std::size_t offset = 0;
        auto offsetItr = axisOffsets.rbegin();
        auto sizeItr = axisSizes.rbegin();
        for (; offsetItr != axisOffsets.rend(); ++offsetItr, ++sizeItr)
        {
            if (*offsetItr > *sizeItr)
                return SIZE_MAX;
            offset += *offsetItr * stride;
            stride *= *sizeItr;
        }
        return offset;
    }

    std::vector<std::size_t> axisOffsets(
        std::size_t offset,
        const std::vector<std::size_t> &axisSizes
    )
    {
        std::vector<std::size_t> strides = ::strides(axisSizes);
        std::vector<std::size_t> offsets(axisSizes.size(), 0);
        for (std::size_t idx = 0; idx < axisSizes.size(); ++idx)
        {
            std::ldiv_t div = std::ldiv(offset, strides[idx]);
            offsets[idx] = div.quot;
            offset = div.rem;
        }
        return offsets;
    }
}

namespace H5Histograms
{
    ArrayIndexer::const_iterator::const_iterator(const std::vector<std::size_t> &axisSizes)
        : m_pos(axisSizes.size(), 0), m_max(axisSizes)
    {
        // If any of these are 0 then the iterator is automatically at the end and should be set as such
        if (std::find(axisSizes.begin(), axisSizes.end(), 0) != axisSizes.end())
            m_pos = endPos(axisSizes);
    }

    ArrayIndexer::const_iterator::const_iterator(
        const std::vector<std::size_t> &axisSizes, const std::vector<std::size_t> &pos)
        : m_pos(pos), m_max(axisSizes)
    {
        if (axisSizes.size() != pos.size())
            throw std::invalid_argument("Dimensions do not match");
    }

    ArrayIndexer::const_iterator ArrayIndexer::const_iterator::createEnd(const std::vector<std::size_t> &axisSizes)
    {
        return const_iterator(axisSizes, endPos(axisSizes));
    }

    std::size_t ArrayIndexer::const_iterator::offset() const
    {
        return ::offset_noCheck(m_pos, m_max);
    }

    bool ArrayIndexer::const_iterator::operator==(const const_iterator &other) const
    {
        return m_pos == other.m_pos && m_max == other.m_max;
    }

    bool ArrayIndexer::const_iterator::operator!=(const const_iterator &other) const
    {
        return !(*this == other);
    }

    ArrayIndexer::const_iterator &ArrayIndexer::const_iterator::operator++()
    {
        auto posItr = m_pos.rbegin();
        auto maxItr = m_max.rbegin();
        for (; posItr != m_pos.rend(); ++posItr, ++maxItr)
        {
            if (++(*posItr) != *maxItr)
                // This is a valid position so stay here
                return *this;
            // Otherwise need to reset this one to 0 and go to the next
            *posItr = 0;
        }
        // If we get here then we've reach the end. Set the position to the correct point
        m_pos = endPos(m_max);
        return *this;
    }

    ArrayIndexer::const_iterator ArrayIndexer::const_iterator::operator++(int)
    {
        const_iterator itr = *this;
        ++*this;
        return itr;
    }

    ArrayIndexer::const_iterator &ArrayIndexer::const_iterator::operator--()
    {
        auto posItr = m_pos.rbegin();
        auto maxItr = m_max.rbegin();
        for (; posItr != m_pos.rend(); ++posItr, ++maxItr)
        {
            if (*posItr != 0)
            {
                // We can decrement this and still be in a valid position so do this;
                --(*posItr);
                return *this;
            }
            // Otherwise set it to the maximum - 1 (i.e. highest valid position) and continue with
            // the next one
            *posItr = *maxItr - 1;
        }
        // If we get here then we're decrementing the start iterator which is undefined behaviour
        // according to the standard. The correct response to UB is to break the program :)
        throw std::runtime_error("Decrementing the start iterator is undefined behaviour!");
        return *this; 
    }

    ArrayIndexer::const_iterator ArrayIndexer::const_iterator::operator--(int)
    {
        const_iterator itr = *this;
        --*this;
        return itr;
    }

    std::vector<std::size_t> ArrayIndexer::const_iterator::endPos(const std::vector<std::size_t> &max)
    {
        // Need to set the end position such that decrementing it gives you the last element
        std::vector<std::size_t> pos;
        pos.reserve(max.size());
        for (std::size_t v : max)
            pos.push_back(v - 1);
        // Increase the last element. Then the decrement operator will just reduce this and it will
        // be in the right place
        ++pos.back();
        return pos;
    }

    ArrayIndexer::ArrayIndexer(const std::vector<std::size_t> &axisSizes)
        : m_axisSizes(axisSizes) {}

    std::vector<std::size_t> ArrayIndexer::strides() const { return ::strides(m_axisSizes); }

    std::size_t ArrayIndexer::nEntries() const {
        return std::accumulate(m_axisSizes.begin(), m_axisSizes.end(), 1, std::multiplies<std::size_t>{});
    }

    std::size_t ArrayIndexer::offset(const std::vector<std::size_t> &axisOffsets) const
    {
        if (axisOffsets.size() != nDims())
            throw std::invalid_argument("Dimensions do not match!");
        return offset_noCheck(axisOffsets);
    }

    std::vector<std::size_t> ArrayIndexer::axisOffsets(std::size_t offset) const
    {
        return ::axisOffsets(offset, m_axisSizes);
    }

    std::size_t ArrayIndexer::offset_noCheck(const std::vector<std::size_t> &axisOffsets) const
    {
        return ::offset_noCheck(axisOffsets, m_axisSizes);
    }

    ArrayIndexer::const_iterator ArrayIndexer::begin() const
    {
        return const_iterator(m_axisSizes);
    }

    ArrayIndexer::const_iterator ArrayIndexer::end() const
    {
        return const_iterator::createEnd(m_axisSizes);
    }
}