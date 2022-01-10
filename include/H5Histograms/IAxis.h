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

#ifndef H5HISTOGRAMS_IAXIS_H
#define H5HISTOGRAMS_IAXIS_H

#include "H5Cpp.h"
#include "H5Composites/IBufferWriter.h"
#include "H5Composites/GenericFactory.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include <string>
#include <variant>
#include <vector>

namespace H5Histograms
{
    class IAxis : virtual public H5Composites::IBufferWriter, virtual public H5Composites::TypeRegister::RegistreeBase
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

        /// The typeID of the axis
        // virtual H5Composites::TypeRegister::id_t getTypeID() const = 0;

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

    using IAxisFactory = H5Composites::GenericFactory<IAxis>;
    using IAxisUPtr = H5Composites::GenericFactoryUPtr<IAxis>;
} //> end namespace H5Histograms

#define H5HISTOGRAMS_REGISTER_IAXIS_FACTORY(AXIS) \
    const bool AXIS::registeredIAxisFactory = H5Histograms::IAxisFactory::instance().registerFactory<AXIS>()

template <>
struct H5Composites::H5DType<std::unique_ptr<H5Histograms::IAxis>>
{
    static H5::DataType getType(const std::unique_ptr<H5Histograms::IAxis> &value);
};

template <>
struct H5Composites::BufferReadTraits<std::unique_ptr<H5Histograms::IAxis>>
{
    static std::unique_ptr<H5Histograms::IAxis> read(const void *buffer, const H5::DataType &dtype);
};

template <>
struct H5Composites::BufferWriteTraits<std::unique_ptr<H5Histograms::IAxis>>
{
    static void write(const std::unique_ptr<H5Histograms::IAxis> &value, void *buffer, const H5::DataType &dtype);
};

#endif //> !H5HISTOGRAMS_IAXIS_H