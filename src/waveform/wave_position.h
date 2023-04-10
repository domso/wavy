#pragma once

#include <vector>
#include "util/bits_to_words.h"

namespace wavy::waveform {
    template<typename T>
    struct wave_position {
        wave_position(const util::bits_to_words<T>& words, const std::vector<uint32_t>& sizes) : m_words(words), m_sizes(sizes) {}
        void reset() {
            m_time = 0;
            m_word_pos = m_words.begin();
            m_size_pos = m_sizes.begin();
        }
        bool move_left() {
            if (m_word_pos != m_words.begin() && m_size_pos != m_sizes.begin()) {
                --m_word_pos;
                --m_size_pos;
                m_time -= *m_size_pos;

                return true;
            }

            return false;
        }
        bool move_right() {
            if (std::distance(m_size_pos, m_sizes.end()) > 1) { 
                m_time += *m_size_pos;
                ++m_size_pos;
                ++m_word_pos;

                return true;
            }

            return false;
        }

        size_t time() const {
            return m_time * 1024;
        }

        size_t duration() const {
            return *m_size_pos * 1024;
        }

        const std::vector<T>& words() const {
            return *m_word_pos;
        }

        void operator=(const wave_position& copy) {
            m_time = copy.m_time;
            m_word_pos = copy.m_word_pos;
            m_size_pos = copy.m_size_pos;
        }
        bool operator==(const wave_position& copy) {
            return m_time == copy.m_time;
        }

    private:
        size_t m_time = 0;

        typename util::bits_to_words<T>::iterator m_word_pos;
        std::vector<uint32_t>::const_iterator m_size_pos;
        
        const util::bits_to_words<T>& m_words;
        const std::vector<uint32_t>& m_sizes;
    };
}
