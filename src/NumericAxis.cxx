#include "H5Histograms/NumericAxis.h"

namespace H5Histograms
{
    NumericAxis::NumericAxis(const std::string &label) : m_label(label) {}

    std::size_t NumericAxis::binOffsetFromValue(const IAxis::value_t &value) const
    {
        return binOffsetFromIndex(std::get<1>(findBin(value)));
    }

    std::size_t NumericAxis::binOffsetFromIndex(const IAxis::index_t &index) const
    {
        std::size_t binIndex = std::get<1>(index);
        if (binIndex >= fullNBins())
            return SIZE_MAX;
        else
            return binIndex;
    }

    bool NumericAxis::containsValue(const IAxis::value_t &value) const
    {
        return std::get<1>(value) <= fullNBins();
    }
}