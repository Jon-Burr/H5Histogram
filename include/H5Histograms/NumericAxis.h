/**
 * @file NumericAxis.h
 * @author Jon Burr
 * @brief Base class for numeric axes
 * @version 0.0.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "H5Histograms/IAxis.h"

namespace H5Histograms
{
    class NumericAxis : public IAxis
    {
    public:
        using index_t = std::size_t;
        using value_t = double;

        NumericAxis(const std::string &label);

        /// The type of this axis
        Type axisType() const override { return Type::Numeric; }

        /// The axis label
        std::string label() const override { return m_label; }

        /// Get the offset of a bin from its value
        std::size_t binOffsetFromValue(const IAxis::value_t &value) const override;

        /// Get the offset of a bin from its index
        std::size_t binOffsetFromIndex(const IAxis::index_t &index) const override;

        /// Whether the axis contains a bin that holds the given value
        bool containsValue(const IAxis::value_t &value) const override;

    protected:
        std::string m_label;
    }; //> end class NumericAxis
}