#include "H5Histograms/IAxis.h"
#include "H5Composites/CompDTypeUtils.h"

namespace H5Composites
{
    H5::DataType H5DType<std::unique_ptr<H5Histograms::IAxis>>::getType(const std::unique_ptr<H5Histograms::IAxis> &value)
    {
        std::vector<std::pair<H5::DataType, std::string>> components;
        components.reserve(2);
        components.emplace_back(getH5DType<TypeRegister::id_t>(), "typeID");
        components.emplace_back(value->h5DType(), "data");
        return createCompoundDType(components);
    }

    std::unique_ptr<H5Histograms::IAxis> BufferReadTraits<std::unique_ptr<H5Histograms::IAxis>>::read(
        const void *buffer, const H5::DataType &dtype)
    {
        H5::CompType compDType(dtype.getId());
        TypeRegister::id_t typeID = readCompositeElement<TypeRegister::id_t>(
            buffer, compDType, "typeID");
        return H5Histograms::IAxisFactory::instance().create(
            typeID,
            getMemberPointer(buffer, compDType, "data"),
            compDType.getMemberDataType(1));
    }

    void BufferWriteTraits<std::unique_ptr<H5Histograms::IAxis>>::write(
        const std::unique_ptr<H5Histograms::IAxis> &value, void *buffer, const H5::DataType &dtype)
    {
        H5::CompType compDType(dtype.getId());
        writeCompositeElement<TypeRegister::id_t>(
            value->getTypeID(), buffer, compDType, "typeID");
        value->writeBufferWithType(
            getMemberPointer(buffer, compDType, "data"),
            compDType.getMemberDataType(compDType.getMemberIndex("data")));
    }
}

namespace H5Histograms
{
    IAxis::ExtensionInfo IAxis::ExtensionInfo::createIdentity(std::size_t oldNBins)
    {
        return ExtensionInfo{[] (std::size_t idx) { return idx; }, oldNBins};
    }

    IAxis::ExtensionInfo IAxis::ExtensionInfo::createShift(std::size_t oldNBins, std::size_t shift)
    {
        return ExtensionInfo{
            [shift] (std::size_t idx) { return idx + shift; },
            oldNBins
        };
    }

    IAxis::ExtensionInfo IAxis::ExtensionInfo::createMapped(const std::vector<std::size_t> &map)
    {
        return ExtensionInfo {
            [map] (std::size_t idx) { return map.at(idx); },
            map.size()
        };
    }
}