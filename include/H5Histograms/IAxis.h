/**
 * @file IAxis.h
 * @author Jon Burr
 * @brief Base class for axes
 * @version 0.0.0
 * @date 2022-01-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "H5Composites/IBufferWriter.h"
#include <string>
#include <variant>
#include <vector>

namespace H5Histograms
{
    class IAxis : public H5Composites::IBufferWriter
    {
    public:
        using index_t = std::variant<std::string, std::size_t>;
        using value_t = std::variant<std::string, double>;
        /**
         * @brief The type of data stored along the axis
         */
        enum class Type
        {
            Category = 0, ///< Category information
            Numeric = 1   ///< Numeric information
        };

        /// The type of this axis
        virtual Type axisType() const = 0;

        /// The axis label
        virtual std::string label() const = 0;

        /// If the axis is extendable
        virtual bool isExtendable() const = 0;

        /// The number of non-overflow bins on the axis
        virtual std::size_t nBins() const = 0;

        /// The number of bins on the axis (including under/overflow)
        virtual std::size_t fullNBins() const = 0;

        /// Get the offset of a bin from its value
        virtual std::size_t binOffsetFromValue(const value_t &value) const = 0;

        /// Get the offset of a bin from its index
        virtual std::size_t binOffsetFromIndex(const index_t &index) const = 0;

        /// Get the index of a bin from its value
        virtual index_t findBin(const value_t &value) const = 0;

        /// Whether the axis contains a bin that holds the given value
        virtual bool containsValue(const value_t &value) const = 0;

        /**
         * @brief Extend the axis to contain a particular value
         * 
         * @param value The value to contain
         * @param[out] offset The offset of the bin containing the specified value
         * @return A mapping of new bin offsets to the old bin offsets
         * 
         * The return value is a vector with one entry per bin in the new array. Each value in this
         * vector is a list of bin numbers in the old array. If this list is empty, the new bin has
         * 0 entries, if it has exactly 1 entry then it copies the entries in that bin. Otherwise 
         * its contents is the sum of the entries in the old bins.
         * 
         * If the axis doesn't need to be extended to accommodate the value the returned vector is empty.
         * This will always be the case if the axis is not extendable.
         */
        virtual std::vector<std::vector<std::size_t>> extendAxis(const value_t &value, std::size_t &offset) = 0;
    }; //> end class IAxis
} //> end namespace H5Histograms