#include "H5Histograms/CategoryAxis.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/FixedLengthStringTraits.h"
#include "H5Composites/FixedLengthVectorTraits.h"
#include "H5Composites/CompDTypeUtils.h"

#include <algorithm>

H5HISTOGRAMS_REGISTER_IAXIS(H5Histograms::CategoryAxis)

namespace H5Histograms
{

    const H5Composites::CompositeDefinition<CategoryAxis> &CategoryAxis::compositeDefinition()
    {
        static H5Composites::CompositeDefinition<CategoryAxis> definition;
        static bool init = false;
        if (!init)
        {
            init = true;
            definition.add<H5Composites::FLString>(&CategoryAxis::m_label, "label");
            definition.add<H5Composites::FLVector<H5Composites::FLString>>(&CategoryAxis::m_categories, "categories");
            definition.add(&CategoryAxis::m_extendable, "extendable");
        }
        return definition;
    }

    CategoryAxis::CategoryAxis(const void *buffer, const H5::DataType &dtype)
    {
        compositeDefinition().readBuffer(*this, buffer, dtype);
    }

    CategoryAxis::CategoryAxis(
        const std::string &label,
        const std::vector<std::string> &categories,
        bool extendable)
        : m_label(label),
          m_categories(categories),
          m_extendable(extendable)
    {
    }

    void CategoryAxis::writeBuffer(void *buffer) const
    {
        compositeDefinition().writeBuffer(*this, buffer);
    }

    H5::DataType CategoryAxis::h5DType() const
    {
        return compositeDefinition().dtype(*this);
    }

    H5Composites::H5Buffer CategoryAxis::mergeBuffers(const std::vector<std::pair<H5::DataType, const void *>> &buffers)
    {
        auto itr = buffers.begin();
        CategoryAxis axis = H5Composites::fromBuffer<CategoryAxis>(itr->second, itr->first);
        for (++itr; itr != buffers.end(); ++itr)
            axis.merge(H5Composites::fromBuffer<CategoryAxis>(itr->second, itr->first));
        return H5Composites::toBuffer(axis);
    }

    void CategoryAxis::merge(const CategoryAxis &other)
    {
        if (m_label != other.m_label)
            throw std::invalid_argument("Axis labels do not match '" + m_label + "' != '" + other.m_label + "'");
        if (m_extendable != other.m_extendable)
            throw std::invalid_argument("Extendable does not match!");
        if (m_extendable)
        {
            // Need to add any categories that are not already present
            for (const std::string &category : other.m_categories)
                if (std::find(m_categories.begin(), m_categories.end(), category) == m_categories.end())
                    m_categories.push_back(category);
        }
        else if (m_categories != other.m_categories)
            throw std::invalid_argument("Categories do not match!");
    }

    std::size_t CategoryAxis::fullNBins() const
    {
        if (m_extendable)
            return m_categories.size();
        else
            return m_categories.size() + 1;
    }

    std::size_t CategoryAxis::binOffsetFromValue(const IAxis::value_t &value) const
    {
        auto itr = std::find(m_categories.begin(), m_categories.end(), std::get<0>(value));
        if (itr == m_categories.end())
        {
            if (m_extendable)
                return SIZE_MAX;
            else
                return m_categories.size();
        }
        else
            return std::distance(m_categories.begin(), itr);
    }

    std::size_t CategoryAxis::binOffsetFromIndex(const IAxis::index_t &index) const
    {
        // index and value are the same for category axes
        return binOffsetFromValue(std::get<0>(index));
    }

    IAxis::index_t CategoryAxis::indexFromBinOffset(std::size_t offset) const
    {
        if (offset >= m_categories.size())
            return overflowName();
        return m_categories[offset];
    }

    IAxis::index_t CategoryAxis::findBin(const IAxis::value_t &value) const
    {
        if (containsValue(value))
            return std::get<0>(value);
        else
            return overflowName();
    }

    bool CategoryAxis::containsValue(const IAxis::value_t &value) const
    {
        return std::find(m_categories.begin(), m_categories.end(), std::get<0>(value)) != m_categories.end();
    }

    IAxis::ExtensionInfo CategoryAxis::extendAxis(
        const IAxis::value_t &variantValue, std::size_t &offset)
    {
        std::size_t oldNBins = nBins();
        std::string value = std::get<0>(variantValue);
        offset = binOffsetFromValue(value);
        if (offset == SIZE_MAX)
        {
            // No appropriate bin exists
            // set the offset to be the new bin
            offset = m_categories.size();
            m_categories.push_back(value);
        }
        // No matter what existing bins get remapped to the same index (the new bin is at the end)
        return ExtensionInfo::createIdentity(oldNBins);
    }

    IAxis::ExtensionInfo CategoryAxis::compareAxis(const IAxis &_other) const
    {
        const CategoryAxis &other = dynamic_cast<const CategoryAxis &>(_other);
        if (m_extendable != other.m_extendable)
            throw std::invalid_argument("Extendable does not match!");
        if (m_categories == other.m_categories)
            return ExtensionInfo::createIdentity(other.fullNBins());
        if (!m_extendable)
            throw std::invalid_argument("Categories do not match on non-extendable axis!");
        std::vector<std::size_t> map;
        map.reserve(other.fullNBins());
        for (const std::string &category : other.m_categories)
        {
            auto itr = std::find(m_categories.begin(), m_categories.end(), category);
            if (itr == m_categories.end())
                throw std::out_of_range("Missing category: " + category);
            map.push_back(std::distance(m_categories.begin(), itr));
        }
        return ExtensionInfo::createMapped(map);
    }
}
