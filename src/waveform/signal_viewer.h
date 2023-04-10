#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>

#include "signal.h"

namespace wavy::waveform {
    template<typename T>
    class signal_viewer {
    public:
        void set_frame(const size_t columns, const size_t rows) {
            m_columns = columns;
            m_rows = rows;

        }
        void add_signal(const size_t index, const size_t max_time, const std::string& name, signal<T>* data) {
            m_selected_signals[index].name = name;
            m_selected_signals[index].trace = data;
            m_selected_signals[index].pos = m_selected_signals.size() - 1;

            m_selected_signals[index].trace->activate(max_time);

            m_max_name_length = std::max(m_max_name_length, name.length());
            m_cursor_column = std::max(m_max_name_length, m_cursor_column);

            for (const auto& [index, data] : m_selected_signals) {
                data.trace->drawn_wave().set_frame(m_columns - m_max_name_length);
            }

            for (const auto& [other_index, align_with_me] : m_selected_signals) {
                if (other_index != index) {
                    m_selected_signals[index].trace->drawn_wave().align_with(align_with_me.trace->drawn_wave());
                    break;
                }
            }

            update_all_frames();
            update_frame();
        }
        void remove_signal(const size_t index) {
            auto pos = m_selected_signals[index].pos;
            for (auto& [index, data] : m_selected_signals) {
                if (data.pos > pos) {
                    data.pos--;
                }
            }
            m_selected_signals[index].trace->deactivate();
            m_selected_signals.erase(index);

            update_all_frames();
            update_frame();
        }
        
        const std::string& frame() const {
            return m_frame;
        }

        void shift_up() {
            if (m_cursor_line > 0 && m_cursor_line < m_selected_signals.size()) {
                for (auto& [index, data] : m_selected_signals) {
                    if (data.pos == m_cursor_line - 1) {
                        data.pos++;
                    } else if (data.pos == m_cursor_line) {
                        data.pos--;
                    }
                }
                
                update_all_frames();
                move_up();
            }
        }
        void shift_down() {
            if (m_cursor_line < m_selected_signals.size() - 1) {
                for (auto& [index, data] : m_selected_signals) {
                    if (data.pos == m_cursor_line + 1) {
                        data.pos--;
                    } else if (data.pos == m_cursor_line) {
                        data.pos++;
                    }
                }
                
                update_all_frames();
                move_down();
            }
        }

        void move_up() {
            if (m_cursor_line > 0) {
                if (m_cursor_line == m_start_line) {
                    m_start_line--;
                }
                m_cursor_line--;
                update_frame();
            }
        }

        void move_down() {
            if (m_cursor_line == m_start_line + m_rows - 1) {
                m_start_line++;
            }
            m_cursor_line++;
            update_frame();
        }

        void move_left() {
            if (m_cursor_column > m_max_name_length) {
                m_cursor_column--;
            } else {
                for (auto& [index, data] : m_selected_signals) {
                    data.trace->drawn_wave().scroll_right(1);
                }
                update_all_frames();
            }

            update_frame();
        }

        void move_right() {
            if (m_cursor_column == m_columns - 1) {
                for (auto& [index, data] : m_selected_signals) {
                    data.trace->drawn_wave().scroll_left(1);
                }
                update_all_frames();
            } else {
                m_cursor_column++;
            }
            update_frame();
        }

        void zoom_in() {
            auto delta = m_cursor_column - m_max_name_length;

            for (auto& [index, data] : m_selected_signals) {
                data.trace->drawn_wave().scroll_left(delta);
                data.trace->drawn_wave().zoom_in();
                data.trace->drawn_wave().scroll_right(delta);
            }

            update_all_frames();
            update_frame();
        }

        void zoom_out() {
            auto delta = m_cursor_column - m_max_name_length;

            for (auto& [index, data] : m_selected_signals) {
                data.trace->drawn_wave().scroll_left(delta);
                data.trace->drawn_wave().zoom_out();
                data.trace->drawn_wave().scroll_right(delta);
            }

            update_all_frames();
            update_frame();
        }
    private:
        void update_all_frames() {
            m_all_frames.resize(m_selected_signals.size());

            for (const auto& [index, data] : m_selected_signals) {
                m_all_frames[data.pos] = {data.name, data.trace->drawn_wave().frame()};
            }
        }
        std::string add_style_to_line(const std::string& s, const size_t line) const {
            if (line == m_cursor_line) {
                return
                    cmd::add_style<cmd::styles::font::bold, cmd::styles::colors::background::extended<123>>(s.substr(0, m_cursor_column)) +
                    cmd::add_style<cmd::styles::font::bold, cmd::styles::colors::background::extended<32>>(s.substr(m_cursor_column, 1)) +
                    cmd::add_style<cmd::styles::font::bold, cmd::styles::colors::background::extended<123>>(s.substr(m_cursor_column + 1));
            } else {
                if ((line % 2) > 0) {
                    return
                        cmd::add_style<cmd::styles::colors::background::extended<240>>(s.substr(0, m_cursor_column)) +
                        cmd::add_style<cmd::styles::colors::background::extended<32>>(s.substr(m_cursor_column, 1)) +
                        cmd::add_style<cmd::styles::colors::background::extended<240>>(s.substr(m_cursor_column + 1));
                } else {
                    return
                        cmd::add_style<cmd::styles::colors::background::extended<241>>(s.substr(0, m_cursor_column)) +
                        cmd::add_style<cmd::styles::colors::background::extended<32>>(s.substr(m_cursor_column, 1)) +
                        cmd::add_style<cmd::styles::colors::background::extended<241>>(s.substr(m_cursor_column + 1));
                }
            }
        }
        std::string replace_bit_symbols(const std::string& s) const {
            std::string result;
            result.reserve(s.length());
            for (const auto c : s) {
                if (c != '#') {
                    result += c;
                } else {
                    result += "‾";
                }
            }

            return result;
        }
        void update_frame() {
            m_frame = "";

            for (auto line = m_start_line; line < std::min(m_all_frames.size(), m_start_line + m_rows); line++) {
                const auto& [name, trace] = m_all_frames[line];
                auto s = std::string(m_max_name_length - name.length(), ' ') + name + trace;
                s = s.substr(0, std::min(m_columns, s.length()));
                s += std::string(m_columns - s.length(), ' ');

                m_frame += replace_bit_symbols(add_style_to_line(s, line)) + "\n";
            }
            for (auto line = std::min(m_all_frames.size(), m_start_line + m_rows); line < m_start_line + m_rows; line++) {
                auto s = std::string(m_columns, ' ') + "\n";

                m_frame += add_style_to_line(s, line);
            }
        }

        struct signal_data {
            std::string name;
            signal<T>* trace;
            size_t pos;
        };

        std::unordered_map<size_t, signal_data> m_selected_signals;
        std::vector<std::pair<std::string, std::string>> m_all_frames;

        size_t m_columns;
        size_t m_rows;
        size_t m_max_name_length = 0;
        std::string m_frame;
        size_t m_start_line = 0;
        size_t m_cursor_line = 0;

        size_t m_cursor_column = 0;
    };
}
