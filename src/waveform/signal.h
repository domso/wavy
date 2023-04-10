#pragma once

#include <vector>
#include <memory>

#include "util/bits_to_words.h"
#include "wave.h"

namespace wavy::waveform {
    template<typename T>
    class signal {
    public:
        void init(const int width) {
            m_converter.reset(width);
        }
        
        void activate(const size_t max_time) {
            auto delta = max_time - m_current_time;
            m_sizes.push_back(delta + 128);

            m_wave = std::make_unique<wave<T>>(m_converter, m_sizes);
        }

        void deactivate() {
            m_sizes.pop_back();
            m_wave = nullptr;
        }

        void add_transition(const std::string& value, const size_t time) {
            T current = 0;
            size_t n = 0;
            std::vector<T> converted;

            for (auto it = value.rbegin(); it != value.rend(); ++it) {
                T v = *it - 48;
                v = v << n;
                current = current | v;
                n++;
                if (n == std::min(sizeof(T) * 8, m_converter.width())) {
                    n = 0;
                    converted.push_back(current);
                    current = 0;
                }
            }
            if (n > 0) {
                converted.push_back(current);
            }

            m_converter.add(converted);

            if (time == 0) {
                m_current_time = time;
            } else {
                auto delta = time - m_current_time;

                m_sizes.push_back(delta);

                m_current_time = time;
            }
        }

        wave<T>& drawn_wave() {
            return *m_wave;
        }
    private:
        util::bits_to_words<T> m_converter;
        std::vector<uint32_t> m_sizes;
        size_t m_current_time = 0;

        std::unique_ptr<wave<T>> m_wave;
    };
}
