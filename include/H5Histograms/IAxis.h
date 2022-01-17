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
#include "H5Composites/MergeFactory.h"
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <functional>

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
        /**
         * @brief Hold information about axis extensions
         */
        struct ExtensionInfo
        {
            std::function<std::size_t(std::size_t)> func;
            std::size_t oldNBins;

            static ExtensionInfo createIdentity(std::size_t oldNBins);
            static ExtensionInfo createShift(std::size_t oldNBins, std::size_t shift=0);
            static ExtensionInfo createMapped(const std::vector<std::size_t> &map);
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

        /// Get the index from a bin offset
        virtual index_t indexFromBinOffset(std::size_t index) const = 0;

        /// Get the index of a bin from its value
        virtual index_t findBin(const value_t &value) const = 0;

        /// Whether the axis contains a bin that holds the given value
        virtual bool containsValue(const value_t &value) const = 0;

        /**
         * @brief Extend the axis to contain a particular value
         * 
         * @param value The value to contain
         * @param[out] offset The offset of the bin containing the specified value
         */
        virtual ExtensionInfo extendAxis(const value_t &value, std::size_t &offset) = 0;

        virtual ExtensionInfo compareAxis(const IAxis &other) const = 0;
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

#define H5HISTOGRAMS_DECLARE_IAXIS() \
    H5COMPOSITES_DECLARE_TYPEID() \
    H5COMPOSITES_DECLARE_MERGE() \
    H5COMPOSITES_DECLARE_GENFACT(IAxis)

#define H5HISTOGRAMS_REGISTER_IAXIS_WITH_NAME(TYPE, NAME) \
    H5COMPOSITES_REGISTER_TYPE_WITH_NAME(TYPE, NAME) \
    H5COMPOSITES_REGISTER_MERGE(TYPE) \
    H5COMPOSITES_REGISTER_GENFACT(IAxis, TYPE)

#define H5HISTOGRAMS_REGISTER_IAXIS(TYPE) \
    H5HISTOGRAMS_REGISTER_IAXIS_WITH_NAME(TYPE, #TYPE)


#endif //> !H5HISTOGRAMS_IAXIS_H