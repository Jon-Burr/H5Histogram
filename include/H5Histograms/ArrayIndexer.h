/**
 * @file ArrayIndexer.h
 * @author Jon Burr
 * @brief Helper class to calculate offsets in a 1D array that represents an ND array
 * @version 0.0.0
 * @date 2022-01-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5HISTOGRAMS_ARRAYINDEXER_H
#define H5HISTOGRAMS_ARRAYINDEXER_H

#include <vector>
#include <iterator>

namespace H5Histograms
{
    /**
     * @brief Helper class to calculate offsets in a 1D array representing an ND array
     * 
     * Uses C-style ordering (row-major)
     */
    class ArrayIndexer
    {
    public:

        class const_iterator
        {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::vector<std::size_t>;
            using pointer = const value_type *;
            using reference = const value_type &;
            using iterator_category = std::bidirectional_iterator_tag;

            /// Create an iterator at the start of the given range
            const_iterator(const std::vector<std::size_t> &axisSizes);

            /// Create an iterator for the given range at the given position
            const_iterator(const std::vector<std::size_t> &axisSizes, const std::vector<std::size_t> &pos);

            /// Create an end iterator for the given axis sizes
            static const_iterator createEnd(const std::vector<std::size_t> &axisSizes);

            /// Dereference the iterator
            reference operator*() const { return m_pos; }

            /// Dereference the iterator
            pointer operator->() const { return &m_pos; }

            /**
             * @brief Get the 1D offset corresponding to this position
             */
            std::size_t offset() const;

            /**
             * @brief Get the number of dimensions for this iterator
             */
            std::size_t nDims() const { return m_pos.size(); }

            /// Check equality between iterators
            bool operator==(const const_iterator &other) const;

            /// Check inequality between iterators
            bool operator!=(const const_iterator &other) const;

            /// Increment the iterator
            const_iterator &operator++();

            /// Increment the iterator
            const_iterator operator++(int);

            /// Decrement the iterator
            const_iterator &operator--();

            /// Decrement the iterator
            const_iterator operator--(int);
        private:
            static std::vector<std::size_t> endPos(const std::vector<std::size_t> &max);
            std::vector<std::size_t> m_pos;
            std::vector<std::size_t> m_max;

        }; //> end class const_iterator

        /**
         * @brief Construct a new Array Indexer object
         * 
         * @param axisSizes The number of bins on each axis
         */
        ArrayIndexer(const std::vector<std::size_t> &axisSizes);

        /// The number of dimensions of the array
        std::size_t nDims() const { return m_axisSizes.size(); }

        /// The sizes of each axis
        const std::vector<std::size_t> &axisSizes() const { return m_axisSizes; }

        /// The strides for each axis
        std::vector<std::size_t> strides() const;

        /// The total number of entries in the array
        std::size_t nEntries() const;

        /**
         * @brief Get the offset in the 1D array
         * 
         * @param axisOffsets The offsets along each axis of the ND array
         * @return The resultant 1D offset
         */
        std::size_t offset(const std::vector<std::size_t> &axisOffsets) const;

        /**
         * @brief Get the offsets along each axis from the offset in the 1D array
         * 
         * @param offset The offset in the 1D array
         * @return The offsets along each axis
         */
        std::vector<std::size_t> axisOffsets(std::size_t offset) const;

        /**
         * @brief Get the offset in the 1D array
         * 
         * @param axisOffsets The offsets along each axis of the ND array
         * @return The resultant 1D offset
         * 
         * Performs no check that the dimensions are correct. Use only in contexts where this is
         * guaranteed
         */
        std::size_t offset_noCheck(const std::vector<std::size_t> &axisOffsets) const;

        /// const_iterator to the start of the array
        const_iterator begin() const;

        /// const_iterator to the end of the array
        const_iterator end() const;
    private:
        std::vector<std::size_t> m_axisSizes;
    };
}

#endif //> !H5HISTOGRAMS_ARRAYINDEXER_H