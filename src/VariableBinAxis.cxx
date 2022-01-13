#include "H5Histograms/VariableBinAxis.h"
#include "H5Composites/StringTraits.h"
#include "H5Composites/VectorTraits.h"
#include <algorithm>

namespace H5Histograms
{
    const H5Composites::CompositeDefinition<VariableBinAxis> &VariableBinAxis::compositeDefinition()
    {
        static H5Composites::CompositeDefinition<VariableBinAxis> definition;
        static bool init = false;
        if (!init)
        {
            definition.add(&VariableBinAxis::m_label, "label");
            definition.add(&VariableBinAxis::m_edges, "edges");
            init = true;
        }
        return definition;
    }

    VariableBinAxis::VariableBinAxis(const std::string &label, const std::vector<double> &edges)
        : NumericAxis(label), m_edges(edges) {}

    VariableBinAxis::VariableBinAxis(const void *buffer, const H5::DataType &dtype) : NumericAxis("")
    {
        compositeDefinition().readBuffer(*this, buffer, dtype);
    }

    H5::DataType VariableBinAxis::h5DType() const
    {
        return compositeDefinition().dtype(*this);
    }

    void VariableBinAxis::writeBuffer(void *buffer) const
    {
        return compositeDefinition().writeBuffer(*this, buffer);
    }

    H5Composites::H5Buffer VariableBinAxis::mergeBuffers(const std::vector<std::pair<H5::DataType, const void *>> &buffers)
    {
        auto itr = buffers.begin();
        VariableBinAxis axis = H5Composites::fromBuffer<VariableBinAxis>(itr->second, itr->first);
        for (++itr; itr != buffers.end(); ++itr)
        {
            VariableBinAxis other = H5Composites::fromBuffer<VariableBinAxis>(itr->second, itr->first);
            if (axis.m_label != other.m_label || axis.m_edges != other.m_edges)
                throw std::invalid_argument("VariableBinAxes do not match!");
        }
        return H5Composites::toBuffer(axis);
    }

    std::size_t VariableBinAxis::nBins() const
    {
        return m_edges.size() - 1;
    }

    std::size_t VariableBinAxis::fullNBins() const
    {
        return m_edges.size() + 1;
    }

    IAxis::index_t VariableBinAxis::findBin(const IAxis::value_t &value) const
    {
        auto itr = std::lower_bound(m_edges.begin(), m_edges.end(), std::get<1>(value));
        return static_cast<std::size_t>(std::distance(m_edges.begin(), itr));
    }

    IAxis::ExtensionInfo VariableBinAxis::extendAxis(const IAxis::value_t &value, std::size_t &offset)
    {
        offset = std::get<1>(findBin(value));
        return ExtensionInfo::createIdentity(nBins());
    }

    IAxis::ExtensionInfo VariableBinAxis::compareAxis(const IAxis &_other) const
    {
        const VariableBinAxis &other = dynamic_cast<const VariableBinAxis &>(_other);
        if (m_edges != other.m_edges)
            throw std::invalid_argument("VariableBinAxes edges do not match!");
        return ExtensionInfo::createIdentity(fullNBins());
    }
} //> end namespace H5Histograms
