#pragma once

#include <chrono>
#include <iomanip>
#include <cmath>
#include <mutex>
#include <iostream>

#include "style.h"
#include "window.h"

namespace cmd {
    class loading_bar {
    public:
        void init(const std::string& name, const int max_steps) {
            m_name = name;
            m_max_steps = max_steps;
            auto [w, h] = window::size();
            m_width = w / 2;
            m_step = 0;
            m_last_update = std::chrono::steady_clock::now();
            m_start = std::chrono::steady_clock::now();
            draw();
            window::cursor::hide();
        }
        void step() {
            std::unique_lock ul(m_mutex);
            m_step++;
            auto t1 = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - m_last_update).count();
            if (duration > 500 || m_step == m_max_steps) {
                draw();
                m_last_update = std::chrono::steady_clock::now();
            }
        }
        void multi_step(const int n) {
            std::unique_lock ul(m_mutex);
            m_step += n;
            auto t1 = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - m_last_update).count();
            if (duration > 500 || m_step == m_max_steps) {
                draw();
                m_last_update = std::chrono::steady_clock::now();
            }
        }
        void finalize() {
            std::string line = m_name;
            line += ": Completed in ";
            line += draw_time(m_start, 1);
            line += repeat_string(" ", m_width + 30);
            std::cout << line << std::endl;
            window::cursor::show();
        }
    private:
        std::string item_text(const float fraction) const {
            std::string item;

            if (fraction < 0.2) {
                item = add_style<styles::colors::background::extended<32>>(" ");
            } else if (fraction < 0.4) {
                item = add_style<styles::colors::background::extended<31>>(" ");
            } else if (fraction < 0.6) {
                item = add_style<styles::colors::background::extended<31>>(" ");
            } else if (fraction < 0.8) {
                item = add_style<styles::colors::background::extended<30>>(" ");
            } else if (fraction < 1.0) {
                item = add_style<styles::colors::background::extended<65>>(" ");
            } else {
                item = add_style<styles::colors::background::extended<29>>(" ");
            }

            return item;
        }
        std::string blank_text(const float) const {
            return add_style<styles::colors::background::extended<238>>(" ");
        }
        std::string repeat_string(const std::string& s, const int n) const {
            std::string line;

            for (int i = 0; i < n; i++) {
                line += s;
            }

            return line;            
        }
        std::string draw_bar() const {
            auto fraction = static_cast<float>(m_step) / m_max_steps;
            auto bar_len = static_cast<int>(m_width * fraction);
            std::string line = m_name + ": [";
            line += repeat_string(item_text(fraction), bar_len);
            line += repeat_string(blank_text(fraction), m_width - bar_len);
            line += "] ";
            return line;
        }
        std::string draw_percentage() const {
            auto fraction = static_cast<float>(m_step) / m_max_steps;
            std::ios_base::fmtflags f(std::cout.flags());
            std::cout << std::fixed << std::setprecision(2) << fraction * 100;
            std::cout.flags(f);
            return "% ";
        }
        std::string draw_time(const std::chrono::steady_clock::time_point tp, const float scale) const {
            auto duration = (std::chrono::steady_clock::now() - tp) * scale;

            int days = std::chrono::duration_cast<std::chrono::days>(duration).count();
            int hours = std::chrono::duration_cast<std::chrono::hours>(duration).count() - days * 24;
            int minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count() - days * 24 - hours * 60;
            int seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count() - days * 24 - hours * 60 - minutes * 60;

            std::string line;

            auto f = [&](int c, char m) {
                if (c > 0) {
                    line += std::to_string(c) + m + " ";
                }
            };
            
            f(days, 'd');
            f(hours, 'h');
            f(minutes, 'm');
            f(seconds, 's');

            if (std::chrono::duration_cast<std::chrono::seconds>(duration).count() == 0) {
                line += "<1s";
            }

            return line;            
        }
        void draw() {
            std::string line;
            line += draw_bar();
            std::cout << line;
            line = draw_percentage();

            if (m_step > 0) {
                if (m_step < m_max_steps) {
                    line += " Estimated: ";
                    line += draw_time(m_start, (1.0 / m_step) * (m_max_steps - m_step));
                } else {
                    line += " Took: ";
                    line += draw_time(m_start, 1);
                }
            }
            line += repeat_string(" ", 30);

            line += "\r";

            std::cout << line << std::flush;
        }

        std::string m_name;
        int m_width;
        int m_step;
        int m_max_steps;
        std::chrono::steady_clock::time_point m_last_update;
        std::chrono::steady_clock::time_point m_start;
        std::mutex m_mutex;
    };
}
