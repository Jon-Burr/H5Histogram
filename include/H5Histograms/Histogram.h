/**
 * @file Histogram.h
 * @author Jon Burr
 * @brief ND histogram implementation
 * @version 0.0.0
 * @date 2022-01-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5HISTOGRAMS_HISTOGRAM_H
#define H5HISTOGRAMS_HISTOGRAM_H

#include "H5Histograms/HistogramBase.h"
#include "H5Composites/CompositeDefinition.h"

namespace H5Histograms
{
    template <typename STORAGE = float>
    class Histogram : public HistogramBase
    {
        friend class H5Composites::CompositeDefinition<Histogram>;
        static const H5Composites::CompositeDefinition<Histogram> &compositeDefinition();

    public:
        Histogram(const void *buffer, const H5::DataType &dtype);
        Histogram(std::vector<std::unique_ptr<IAxis>> &&axes);

        H5::DataType h5DType() const override;
        void writeBuffer(void *buffer) const override;

        void fill(const value_t &values, STORAGE weight = 1);

        STORAGE &contents(const index_t &indices);

        STORAGE contents(const index_t &indices) const;

        STORAGE &sumW2(const index_t &indices);

        STORAGE sumW2(const index_t &indices) const;

        std::size_t nEntries() const { return m_nEntries; }

    private:
        void resize(const std::vector<std::vector<std::vector<std::size_t>>> &axisExtensions);

        std::size_t m_nEntries;
        std::vector<STORAGE> m_counts;
        std::vector<STORAGE> m_sumW2;
    }; //> end class Histogram<STORAGE>

    using IntHistogram = Histogram<int>;
    using UIntHistogram = Histogram<unsigned int>;
    using CharHistogram = Histogram<char>;
    using SCharHistogram = Histogram<signed char>;
    using UCharHistogram = Histogram<unsigned char>;
    using ShortHistogram = Histogram<short>;
    using UShortHistogram = Histogram<unsigned short>;
    using LongHistogram = Histogram<long>;
    using LLongHistogram = Histogram<long long>;
    using ULongHistogram = Histogram<unsigned long>;
    using ULLongHistogram = Histogram<unsigned long long>;
    using FloatHistogram = Histogram<float>;
    using DoubleHistogram = Histogram<double>;
} //> end namespace H5Histograms

#endif //> !H5HISTOGRAMS_HISTOGRAM_H