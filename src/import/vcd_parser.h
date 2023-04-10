#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace wavy::import {
    template<typename T>
    class vcd_parser {
    public:
        std::vector<std::string> parse_line(const std::string& s) {
            auto line = split_by_space(s);

            if (!line.empty()) {
                if (!m_header_complete) {
                    parse_header(line);
                } else {
                    parse_data(line);
                }
            }

            return line;
        }

        void set_sink(T& sink) {
            m_sink = &sink;
        }
    private:
        std::vector<std::string> split_by_space(const std::string& s) const {
            std::vector<std::string> result;

            int start = 0;
            int n = 0;

            for (const auto& c : s) {
                if (c == ' ') {
                    auto sub = s.substr(start, n);
                    if (sub.length() > 0) {
                        result.push_back(sub);
                    }
                     
                    start += n + 1;
                    n = 0;
                } else {
                    n++;
                }
            }
            auto sub = s.substr(start);
            if (sub.length() > 0) {
                result.push_back(sub);
            }

            return result;
        }

        void parse_header(const std::vector<std::string>& line) {
            if (line[0] == "$enddefinitions") {
                m_header_complete = true;
            }

            if (line.size() >= 6 && line[0] == "$var" && line[1] == "wire") {
                auto index = m_sink->add_signal(line[4], std::stoll(line[2]));
                m_signal_names[line[3]] = {index, line[4]};
            }
        }

        void parse_data(const std::vector<std::string>& line) {
            if (line[0].length() > 1) {
                if (line[0].at(0) == '#') {
                    m_current_time = std::stoll(line[0].substr(1));
                    return;
                }
                if (line[0].at(0) == 'b') {
                    auto val = line[0].substr(1);
                    const auto& [index, name] = m_signal_names[line[1]];
                    m_sink->add_transition(index, val, m_current_time);
                    return;
                }
                if (line[0].at(0) == '0') {
                    auto val = "0";
                    const auto& [index, name] = m_signal_names[line[0].substr(1)];
                    m_sink->add_transition(index, val, m_current_time);
                    return;
                }
                if (line[0].at(0) == '1') {
                    auto val = "1";
                    const auto& [index, name] = m_signal_names[line[0].substr(1)];
                    m_sink->add_transition(index, val, m_current_time);
                    return;
                }

            }
        }

        bool m_header_complete = false;
        size_t m_current_time = 0;

        std::unordered_map<std::string, std::pair<size_t, std::string>> m_signal_names;
        T* m_sink = nullptr;
    };
}
