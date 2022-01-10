#include "H5Histograms/FixedBinAxis.h"
#include "H5Composites/EnumUtils.h"
#include "H5Composites/StringTraits.h"
#include "H5Composites/VectorTraits.h"

H5COMPOSITES_DEFINE_ENUM_DTYPE(H5Histograms::FixedBinAxis::ExtensionType, NoExtension, PreserveNBins, PreserveBinWidth)

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
                return 0;
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

    std::vector<std::vector<std::size_t>> FixedBinAxis::extendAxis(const IAxis::value_t &value, std::size_t &offset)
    {
        // first figure out if this falls inside the range
        std::size_t bin = std::get<1>(findBin(value));
        if (bin != SIZE_MAX)
        {
            // This is already in the range so don't extend
            offset = bin;
            return {};
        }
        int idx = (std::get<1>(value) - m_min) / binWidth();
        switch (m_extension)
        {
        case ExtensionType::PreserveNBins:
            throw std::invalid_argument("Preserve nBins not implemented yet!");
        case ExtensionType::PreserveBinWidth:
            if (idx < 0)
            {
                // Need to add bins below
                std::vector<std::vector<std::size_t>> newBins(nBins() + std::abs(idx));
                // Shift all current bins up
                for (std::size_t ii = 0; ii < nBins(); ++ii)
                    newBins.at(ii + std::abs(idx)) = {ii};
                // Now change the internal parameters
                m_min -= binWidth() * std::abs(idx);
                m_nBins += std::abs(idx);
                // New value is in the lowest bin
                offset = 0;
                return newBins;
            }
            else
            {
                // Need to add bins above
                std::vector<std::vector<std::size_t>> newBins(nBins() + idx);
                // Keep all current bins where they are
                for (std::size_t ii = 0; ii < nBins(); ++ii)
                    newBins.at(ii) = {ii};
                // Now change the internal parameters
                m_max += binWidth() * idx;
                m_nBins += idx;
                // New value is in the highest bin
                offset = nBins() - 1;
                return newBins;
            }
        default:
            throw std::logic_error("Invalid extension type");
        }
        return {};
    }

    double FixedBinAxis::binWidth() const
    {
        return (m_max - m_min) / m_nBins;
    }
}