#pragma once

#include <stdint.h>
#include <vector>

namespace wavy::util {
    template<typename T>
    class bits_to_words {
    public:
        void reset(const size_t width) {
            m_width = width;
            m_words.clear();
            m_current_insert_offset = 0;

            m_num_words = m_width / (sizeof(T) * 8);
            if ((m_width % (sizeof(T) * 8)) > 0) {
                m_num_words++;
            }
        }   
        const std::vector<T>& data() const {
            return m_words;
        }   

        size_t width() const {
            return m_width;
        }

        void add(const std::vector<T>& word) {
            int remaining = m_width;
            size_t word_index = 0;

            while (remaining > 0) {
                auto offset = std::min(remaining, static_cast<int>(sizeof(T) * 8));
                remaining -= offset;
                
                if (word_index < word.size()) {
                    add_word(word[word_index], offset);
                    word_index++;
                }
            }

            m_number_of_items++;
        }

        size_t num_words() const {
            return m_num_words;
        }

        class iterator {
        public:
            iterator() {}
            iterator(const std::vector<T>& data, const int width, const size_t item, const size_t number_of_items) : m_words(&data), m_width(width), m_item(item), m_number_of_items(number_of_items) {
                m_num_words = m_width / (sizeof(T) * 8);
                if ((m_width % (sizeof(T) * 8)) > 0) {
                    m_num_words++;
                }

                m_current.resize(m_num_words);
                next_word();
            }
            bool operator==(const iterator& it) const {
                return m_item == it.m_item;
            }
            bool operator!=(const iterator& it) const {
                return m_item != it.m_item;
            }
            bool operator<(const iterator& it) const {
                return m_item < it.m_item;
            }
            bool operator>(const iterator& it) const {
                return m_item > it.m_item;
            }
            bool operator<=(const iterator& it) const {
                return m_item <= it.m_item;
            }
            bool operator>=(const iterator& it) const {
                return m_item >= it.m_item;
            }

            iterator operator++(int) {
                auto copy = *this;
                next_word();
                return copy;
            }
            iterator operator--(int) {
                auto copy = *this;
                prev_word();
                return copy;
            }
            iterator& operator++() {
                next_word();
                return *this;
            }
            iterator& operator--() {
                prev_word();
                return *this;
            }

            const std::vector<T>& operator*() const {
                return m_current;
            }
        private:
            void next_word() {
                if (m_item < m_number_of_items) {
                    auto remaining = m_width;
                    size_t dest_bit_counter = 0;
                    size_t dest_word_counter = 0;

                    for (auto& w : m_current) {
                        w = 0;
                    }

                    while (remaining > 0) {
                        T tmp;
                        size_t set_bits = 0;

                        if (m_bit_counter == 0) {
                            m_stored_word = m_words->at(m_word_counter);
                            m_word_counter++;
                            m_bit_counter = sizeof(T) * 8;
                        }

                        tmp = m_stored_word;
                        set_bits = std::min(m_bit_counter, remaining);
                        m_bit_counter -= set_bits;
                        if (set_bits == sizeof(T) * 8) {
                            m_stored_word = 0;
                        } else {
                            m_stored_word = m_stored_word >> set_bits;
                        }
                        remaining -= set_bits;

                        int fit_into_current_word = std::min(set_bits, sizeof(T) * 8 - dest_bit_counter);
                        int fit_into_next_word = set_bits - fit_into_current_word;

                        T store_in_current_word;            
                        if (fit_into_current_word < 64) {
                            store_in_current_word = (tmp & ((1UL << fit_into_current_word) - 1)) << dest_bit_counter;
                        } else {
                            store_in_current_word = tmp << dest_bit_counter;
                        }
                        
                        m_current[dest_word_counter] = m_current[dest_word_counter] | store_in_current_word;
                        dest_bit_counter += fit_into_current_word;

                        if (dest_bit_counter == sizeof(T) * 8) {
                            dest_bit_counter = fit_into_next_word;
                            dest_word_counter++;
                            m_current[dest_word_counter] = ((tmp >> fit_into_current_word) & ((1UL << fit_into_next_word) - 1));
                        }
                    }
                    m_item++;
                }
            }

            void prev_word() {
                if (m_item > 0) {
                    m_item--;
                    int remaining = 2 * m_width;

                    auto remove_from_current = std::min(remaining, static_cast<int>(sizeof(T) * 8 - m_bit_counter));
                    remaining -= remove_from_current;
                    m_bit_counter += remove_from_current;

                    while (remaining > 0) {
                        if (m_word_counter > 0) {
                            m_word_counter--;
                        }

                        auto remove_from_word = std::min(remaining, static_cast<int>(sizeof(T) * 8));
                        remaining -= remove_from_word;
                        m_bit_counter = remove_from_word;
                    }

                    if (m_word_counter > 0) {
                        m_word_counter--;
                    }
                    
                    m_stored_word = m_words->at(m_word_counter);
                    m_stored_word = m_stored_word >> (sizeof(T) * 8 - m_bit_counter);
                    m_word_counter++;
                    
                    next_word();
                    m_item--;
                }
            }
            const std::vector<T>* m_words = nullptr;
            std::vector<T> m_current;

            int m_width;
            int m_num_words;
            int m_bit_counter = 0;
            size_t m_word_counter = 0;
            size_t m_item = 0;
            size_t m_number_of_items;
            T m_stored_word;
        };

        iterator begin() const {
            return iterator(m_words, m_width, 0, m_number_of_items);
        }
        iterator end() const {
            return iterator(m_words, m_width, m_number_of_items, m_number_of_items);
        }
    private:
        T get_current() {
            if (m_words.empty()) {
                m_words.push_back(0);
                return 0;
            }   
            return *m_words.rbegin();
        }   
        void set_current(const size_t c) {
            *m_words.rbegin() = static_cast<T>(c);
        }   
        void set_next(const size_t c) {
            m_words.push_back(static_cast<T>(c));
        }   
        void add_word(const T value, const size_t n) {
            auto data_width = 8 * sizeof(T);
            size_t mask = 1UL << n;
            mask = mask - 1;

            if (n == 64) {
                mask = ~0;
            }

            if (m_current_insert_offset == data_width) {
                m_current_insert_offset = 0;
                set_next(0);
            }   

            auto current = get_current();
            current |= ((value & mask) << m_current_insert_offset);
            m_current_insert_offset += n;
            set_current(current);

            while (m_current_insert_offset > data_width) {
                m_current_insert_offset -= data_width;
                set_next(value >> (n - m_current_insert_offset));
            }   
        }   

        std::vector<T> m_words;
        size_t m_width;
        size_t m_num_words;
        size_t m_current_insert_offset = 0;
        size_t m_number_of_items = 0;
    };  
}
