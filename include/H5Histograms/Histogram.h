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

#include "H5Histograms/ArrayIndexer.h"
#include "H5Histograms/HistogramBase.h"
#include "H5Composites/CompositeDefinition.h"

#include <type_traits>
#include <iterator>
#include <optional>
#include <tuple>

namespace H5Histograms
{
    template <typename STORAGE>
    class Histogram : public HistogramBase
    {
        friend class H5Composites::CompositeDefinition<Histogram>;
        static const H5Composites::CompositeDefinition<Histogram> &compositeDefinition();

    public:
        template <bool CONST>
        class Iterator {
            friend class Iterator<!CONST>;
        public:
            using store_t = std::conditional_t<CONST, STORAGE, STORAGE &>;
            using difference_type = std::ptrdiff_t;
            using value_type = std::tuple<store_t, store_t>;
            using pointer = std::conditional_t<CONST, const value_type *, value_type *>;
            using reference = std::conditional_t<CONST, const value_type &, value_type &>;
            using iterator_category = std::forward_iterator_tag;
            using histogram_t = std::conditional_t<CONST, const Histogram, Histogram>;
            
            /// Create an iterator to the start of a histogram
            Iterator(histogram_t &histogram);

            /// Create an iterator to a particular bin in a histogram
            Iterator(histogram_t &histogram, const Histogram::value_t &values);

            /// Create an end iterator for the given histogram
            static Iterator createEnd(histogram_t &histogram);

            /// Allow converting a non-const iterator to a const iterator
            Iterator(const Iterator<false> &other);

            /// Dereference ths iterator
            reference operator*() { return *m_value; }

            /// Dereference this iterator
            pointer operator->() { return &(*m_value); }

            /// Get the underlying index iterator
            ArrayIndexer::const_iterator idxItr() const { return m_idxItr; }

            /// Get the current bin indices
            HistogramBase::index_t indices() const;

            /// Get the contents of the bin pointed to
            store_t contents() { return std::get<0>(**this); }

            /// Get the sumW2 of the bin pointed to
            store_t sumW2() { return std::get<1>(**this); }

            /// Check equality between iterators
            bool operator==(const Iterator &other) const;

            /// Check inequality between iterators
            bool operator!=(const Iterator &other) const;

            /// Increment the iterator
            Iterator &operator++();

            /// Increment the iterator
            Iterator operator++(int);

            /// Decrement the iterator
            Iterator &operator--();

            /// Decrement the iterator
            Iterator operator--(int);

        private:
            ArrayIndexer::const_iterator m_idxItr;
            histogram_t &m_histo;
            std::size_t m_offset;
            std::optional<value_type> m_value;
        };

        using const_iterator = Iterator<true>;
        using iterator = Iterator<false>;

        Histogram(const void *buffer, const H5::DataType &dtype);
        Histogram(std::vector<std::unique_ptr<IAxis>> &&axes);

        template <typename... AXES>
        static Histogram create(const AXES &... axes)
        {
            std::vector<std::unique_ptr<IAxis>> ptrs;
            ptrs.reserve(sizeof...(axes));
            (ptrs.push_back(std::make_unique<AXES>(axes)), ...);
            return std::move(ptrs);
        }

        H5::DataType h5DType() const override;
        void writeBuffer(void *buffer) const override;

        void fill(const value_t &values, STORAGE weight = 1);

        STORAGE &contents(const index_t &indices);

        STORAGE contents(const index_t &indices) const;

        STORAGE &sumW2(const index_t &indices);

        STORAGE sumW2(const index_t &indices) const;

        std::size_t nEntries() const { return m_nEntries; }

        iterator begin() { return iterator(*this); }

        iterator end() { return iterator::createEnd(*this); }

        const_iterator begin() const { return const_iterator(*this); }

        const_iterator end() const { return const_iterator::createEnd(*this); }

        Histogram &operator+=(const Histogram &h);
    private:
        void resize(const std::vector<IAxis::ExtensionInfo> &axisExtensions);

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