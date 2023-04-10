#pragma once

#include <vector>
#include "util/bits_to_words.h"

#include "wave_position.h"

namespace wavy::waveform {
    template<typename T>
    class wave {
    public:
        wave(const util::bits_to_words<T>& words, const std::vector<uint32_t>& sizes) : m_words(words), m_sizes(sizes), m_start(m_words, m_sizes), m_end(m_words, m_sizes) {
            m_start.reset();
            m_end = m_start;
        }

        void set_frame(const size_t columns) {
            auto duration = columns * m_scale;

            if (duration != m_draw_duration) {
                set_draw_time(m_draw_start_time, duration, true);
            }
        }

        void set_draw_time(const size_t draw_start_time, const size_t draw_duration, const bool force_update) {
            m_draw_start_time = draw_start_time;
            m_draw_duration = draw_duration;

            bool update_frame = force_update;

            while (m_start.time() + m_start.duration() <= draw_start_time && m_start.move_right()) {
                update_frame = true;
            }
            while (draw_start_time < m_start.time() && m_start.move_left()) {
                update_frame = true;
            }

            while (m_end.time() + m_end.duration() < draw_start_time + draw_duration && m_end.move_right()) {
                update_frame = true;
            }
            
            while (draw_start_time + draw_duration < m_end.time() && m_end.move_left()) {
                update_frame = true;
            }

            if (update_frame) {
                m_frame = draw_frame(m_start, m_end);
            }
            update_current_frame();
        }

        void scroll_left(const size_t n) {
            set_draw_time(m_draw_start_time + m_scale * n, m_draw_duration, false);
        }

        void scroll_right(const size_t n) {
            if (m_scale * n <= m_draw_start_time) {
                set_draw_time(m_draw_start_time - m_scale * n, m_draw_duration, false);
            } else {
                set_draw_time(0, m_draw_duration, false);
            }
        }

        void align_with(const wave<T>& other) {
            m_scale = other.m_scale;
            m_draw_start_time = other.m_draw_start_time;
            m_draw_duration = other.m_draw_duration;

            m_start.reset();
            m_end = m_start;

            set_draw_time(m_draw_start_time, m_draw_duration, true);
        }

        void zoom_in() {
            if (m_scale > 1) {
                m_scale /= 2;

                m_draw_duration /= 2;
            }
            m_start.reset();
            m_end = m_start;

            set_draw_time(m_draw_start_time, m_draw_duration, true);
        }

        void zoom_out() {
            m_scale *= 2;
            m_draw_duration *= 2;

            m_start.reset();
            m_end = m_start;

            set_draw_time(m_draw_start_time, m_draw_duration, true);
        }

        const std::string& frame() const {
            return m_current_frame;
        }
        void update_current_frame() {
            size_t end_time_of_data = m_end.time() + m_end.duration();
            size_t end_time_of_draw = m_draw_start_time + m_draw_duration;

            size_t start_time_of_data = m_start.time();
            size_t start_time_of_draw = m_draw_start_time;

            size_t remove_from_end = 0;
            size_t add_to_end = 0;

            size_t remove_from_start = 0;
            size_t add_to_start = 0;

            if (end_time_of_data < end_time_of_draw) {
                add_to_end = (end_time_of_draw - end_time_of_data) / m_scale;
            }
            if (end_time_of_draw < end_time_of_data) {
                remove_from_end = (end_time_of_data - end_time_of_draw) / m_scale;
            }

            if (start_time_of_data < start_time_of_draw) {
                remove_from_start = (start_time_of_draw - start_time_of_data) / m_scale;
            }
            if (start_time_of_draw < start_time_of_data) {
                add_to_start = (start_time_of_data - start_time_of_draw) / m_scale;
            }

            auto extended_frame = m_frame + std::string(add_to_end, ' ');
            m_current_frame = std::string(add_to_start, ' ') + extended_frame.substr(remove_from_start, extended_frame.length() - remove_from_start - remove_from_end);
        }

        std::string draw_frame(const wave_position<T>& start, const wave_position<T>& end) {
            std::string result;

            wave_position<T> current_pos = start;
            while (true) {
                auto s = align_time(current_pos.time());
                auto e = align_time(current_pos.time() + current_pos.duration());
                auto duration = e - s;
                int len = duration / m_scale;

                if (len > 0) {
                    if (m_words.width() > 1) {
                        result += draw_single_fragment(current_pos.words(), len);
                    } else {
                        result += draw_single_bit(current_pos.words()[0], len);
                    }
                }
                
                if (current_pos == end) {
                    return result;
                }
                current_pos.move_right();
            }
        }

        size_t& scale() {
            return m_scale;
        }
    private:
        size_t align_time(const size_t time) const {
            auto m = time % m_scale;

            if (m == 0) {
                return time;
            } else {
                return time - m + m_scale;
            }
        }
        
        std::string draw_single_fragment(const std::vector<T>& words, const uint32_t len) const {
            std::stringstream sstream;
            for (auto it = words.rbegin(); it != words.rend(); ++it) {
                sstream << std::hex << *it;
            }
            std::string label = sstream.str();

            if (len <= 1) {
                return "|";
            }

            if (len < 1 + label.length()) {
                int show_label_length = std::max(0, static_cast<int>(len) - 4);
                if (show_label_length > 0) {
                    return "|" + std::string(std::min(3U, len - 1), '.') + label.substr(label.length() - show_label_length, show_label_length);
                } else {
                    return "|" + std::string(std::min(3U, len - 1), '.');
                }
            } else {
                int space = len - 1 - label.length();
                int left_space = space / 2;
                int right_space = space - left_space;

                return "|" + std::string(left_space, ' ') + label + std::string(right_space, ' ');
            }

            return std::string(len, 'X');
        }

        std::string draw_single_bit(const int s, const uint32_t len) {
            if (len <= 1) {
                return "|";
            }

            if (s == 0) {
                std::string result(len - 1, '_');
                if (m_last == 1) {
                    m_last = 0;
                    return "\\" + result;
                }
                return "_" + result;
            } else {
                std::string result(len - 1, '#');
                if (m_last == 0) {
                    m_last = 1;
                    return "/" + result;
                }
                return "#" + result;
            }
        }

        std::string m_frame;
        std::string m_current_frame;
        int m_last = 1;

        const util::bits_to_words<T>& m_words;
        const std::vector<uint32_t>& m_sizes;

        wave_position<T> m_start;
        wave_position<T> m_end;

        size_t m_draw_start_time = 0;
        size_t m_draw_duration;
        size_t m_scale = 1024;
    };
}
