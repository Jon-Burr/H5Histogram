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
        return std::distance(m_edges.begin(), itr);
    }

    std::vector<std::vector<std::size_t>> VariableBinAxis::extendAxis(const IAxis::value_t &value, std::size_t &offset)
    {
        offset = std::get<1>(findBin(value));
        return {};
    }
} //> end namespace H5Histograms
