#include "H5Histograms/CategoryAxis.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/StringTraits.h"
#include "H5Composites/VectorTraits.h"
#include "H5Composites/CompDTypeUtils.h"

#include <algorithm>

#include <iostream>

namespace H5Histograms
{
    const H5Composites::CompositeDefinition<CategoryAxis> &CategoryAxis::compositeDefinition()
    {
        static H5Composites::CompositeDefinition<CategoryAxis> definition;
        static bool init = false;
        if (!init)
        {
            init = true;
            definition.add(&CategoryAxis::m_label, "label");
            definition.add(&CategoryAxis::m_categories, "categories");
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

    std::vector<std::vector<std::size_t>> CategoryAxis::extendAxis(
        const IAxis::value_t &variantValue, std::size_t &offset)
    {
        std::string value = std::get<0>(variantValue);
        offset = binOffsetFromValue(value);
        if (offset != SIZE_MAX)
            // This means that an appropriate axis exists
            return {};
        // Put the new bin at the end of the existing vector
        std::vector<std::vector<std::size_t>> newBins;
        newBins.reserve(nBins() + 1);
        for (std::size_t idx = 0; idx < nBins(); ++idx)
            newBins.push_back({idx});
        // Now add a new bin to the end
        newBins.push_back({});
        // set the offset to be the new bin
        offset = m_categories.size();
        m_categories.push_back(value);
        return newBins;
    }
}