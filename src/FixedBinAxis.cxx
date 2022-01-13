#include "H5Histograms/FixedBinAxis.h"
#include "H5Composites/EnumUtils.h"
#include "H5Composites/StringTraits.h"
#include "H5Composites/VectorTraits.h"

#include <stdexcept>
#include <cmath>

H5COMPOSITES_DEFINE_ENUM_DTYPE(H5Histograms::FixedBinAxis::ExtensionType, NoExtension, PreserveNBins, PreserveBinWidth)

namespace {

    long nBins(double binDiff, double binWidth)
    {
        double integral;
        double fractional = std::modf(binDiff/binWidth, &integral);
        if (fractional != 0)
            throw std::invalid_argument("Bin edges not compatible");
        return static_cast<long>(integral);
    }

    std::pair<long, long> nBelowAbove(const H5Histograms::FixedBinAxis &lhs, const H5Histograms::FixedBinAxis &rhs)
    {
        if (lhs.binWidth() != rhs.binWidth())
            throw std::invalid_argument("Bin widths do not match!");
        return std::make_pair(nBins(lhs.min() - rhs.min(), lhs.binWidth()), nBins(lhs.max() - rhs.max(), rhs.binWidth()));
    }
}

namespace H5Histograms
{
    const H5Composites::CompositeDefinition<FixedBinAxis> &FixedBinAxis::compositeDefinition()
    {
        static H5Composites::CompositeDefinition<FixedBinAxis> definition;
        static bool init = false;
        if (!init)
        {
            definition.add(&FixedBinAxis::m_label, "label");
            definition.add(&FixedBinAxis::m_nBins, "nBins");
            definition.add(&FixedBinAxis::m_min, "min");
            definition.add(&FixedBinAxis::m_max, "max");
            definition.add(&FixedBinAxis::m_extension, "extension");
            init = true;
        }
        return definition;
    }

    FixedBinAxis::FixedBinAxis(
        const std::string &label,
        std::size_t nBins,
        double min,
        double max,
        ExtensionType extension)
        : NumericAxis(label),
          m_nBins(nBins),
          m_min(min),
          m_max(max),
          m_extension(extension)
    {
    }

    FixedBinAxis::FixedBinAxis(const void *buffer, const H5::DataType &dtype)
        : NumericAxis("")
    {
        compositeDefinition().readBuffer(*this, buffer, dtype);
    }

    H5::DataType FixedBinAxis::h5DType() const
    {
        return compositeDefinition().dtype(*this);
    }

    void FixedBinAxis::writeBuffer(void *buffer) const
    {
        compositeDefinition().writeBuffer(*this, buffer);
    }

    H5Composites::H5Buffer FixedBinAxis::mergeBuffers(const std::vector<std::pair<H5::DataType, const void *>> &buffers)
    {
        auto itr = buffers.begin();
        FixedBinAxis axis = H5Composites::fromBuffer<FixedBinAxis>(itr->second, itr->first);
        for (++itr; itr != buffers.end(); ++itr)
            axis.merge(H5Composites::fromBuffer<FixedBinAxis>(itr->second, itr->first));
        return H5Composites::toBuffer(axis);
    }

    void FixedBinAxis::merge(const FixedBinAxis &other)
    {
        if (m_label != other.m_label)
            throw std::invalid_argument("Axis labels do not match '" + m_label + "' != '" + other.m_label + "'");
        if (m_extension != other.m_extension)
            throw std::invalid_argument("Extension does not match!");
        if (m_nBins == other.m_nBins && m_min == other.m_min && m_max == other.m_max)
            return;
        switch(m_extension)
        {
        case ExtensionType::NoExtension:
            throw std::invalid_argument("Parameters do not match on non-extendable axis");
        case ExtensionType::PreserveNBins:
            throw std::logic_error("Not implemented!");
        case ExtensionType::PreserveBinWidth:
        {
            std::pair<long, long> nBelowAbove = ::nBelowAbove(*this, other);
            if (nBelowAbove.first > 0)
            {
                // our min is more than theirs
                m_min -= nBelowAbove.first * binWidth();
                m_nBins += nBelowAbove.first;
            }
            if (nBelowAbove.second < 0)
            {
                // our max is less than theirs
                m_max += std::abs(nBelowAbove.second) * binWidth();
                m_nBins += std::abs(nBelowAbove.second);
            }
        }
        default:
            throw std::logic_error("Invalid enum value");
        }
    }

    std::size_t FixedBinAxis::fullNBins() const
    {
        if (isExtendable())
            return nBins();
        else
            return nBins() + 2;
    }

    IAxis::index_t FixedBinAxis::findBin(const IAxis::value_t &variantValue) const
    {
        double value = std::get<1>(variantValue);
        // first figure out if this falls inside the range
        int idx = (value - m_min) / binWidth();
        if (idx < 0)
        {
            // Falls below the range
            if (isExtendable())
                // There is no bin for this yet so return SIZE_MAX
                return SIZE_MAX;
            else
                // Goes into the underflow bin
                return static_cast<std::size_t>(0);
        }
        else if (idx > m_nBins)
        {
            // Falls above the range
            if (isExtendable())
                // There is no bin for this yet so return SIZE_MAX
                return SIZE_MAX;
            else
                // Goes into the overflow bin
                return m_nBins + 1;
        }
        else
        {
            if (isExtendable())
                // No overflow/underflow bin so it's a direct conversion
                return static_cast<std::size_t>(idx);
            else
                // Index '0' is reserved for underflow so bump up all numbers by 1
                return static_cast<std::size_t>(idx + 1);
        }
    }

    IAxis::ExtensionInfo FixedBinAxis::extendAxis(const IAxis::value_t &value, std::size_t &offset)
    {
        // first figure out if this falls inside the range
        std::size_t bin = std::get<1>(findBin(value));
        std::size_t oldNBins = nBins();
        if (bin != SIZE_MAX)
        {
            // This is already in the range so don't extend
            offset = bin;
            return ExtensionInfo::createIdentity(oldNBins);
        }
        int idx = (std::get<1>(value) - m_min) / binWidth();
        switch (m_extension)
        {
        case ExtensionType::PreserveNBins:
            throw std::logic_error("Preserve nBins not implemented yet!");
        case ExtensionType::PreserveBinWidth:
            if (idx < 0)
            {
                // Change the internal parameters
                m_min -= binWidth() * std::abs(idx);
                m_nBins += std::abs(idx);
                // New value is in the lowest bin
                offset = 0;
                // Need to add bins below
                return ExtensionInfo::createShift(oldNBins, std::abs(idx));
            }
            else
            {
                // Now change the internal parameters
                m_max += binWidth() * idx;
                m_nBins += idx;
                // New value is in the highest bin
                offset = nBins() - 1;
                // Bins are created above so old indices stay the same
                return ExtensionInfo::createIdentity(oldNBins);
            }
        default:
            throw std::logic_error("Invalid extension type");
        }
        return {};
    }

    IAxis::ExtensionInfo FixedBinAxis::compareAxis(const IAxis &_other) const
    {
        const FixedBinAxis &other = dynamic_cast<const FixedBinAxis &>(_other);
        if (m_extension != other.m_extension)
            throw std::invalid_argument("Extension does not match!");
        if (m_nBins == other.m_nBins && m_min == other.m_min && m_max == other.m_max)
            return ExtensionInfo::createIdentity(fullNBins());
        switch(m_extension)
        {
        case ExtensionType::NoExtension:
            throw std::invalid_argument("Parameters do not match on non-extendable axis");
        case ExtensionType::PreserveNBins:
            throw std::logic_error("Not implemented!");
        case ExtensionType::PreserveBinWidth:
        {
            std::pair<long, long> nBelowAbove = ::nBelowAbove(*this, other);
            if (nBelowAbove.first > 0 || nBelowAbove.second < 0)
                throw std::invalid_argument("Other axis extends further than this one");
            return ExtensionInfo::createShift(std::abs(nBelowAbove.first));
        }
        default:
            throw std::logic_error("Invalid enum value");
        }
                
    }

    double FixedBinAxis::binWidth() const
    {
        return (m_max - m_min) / m_nBins;
    }
}