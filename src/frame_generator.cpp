#include "frame_generator.h"

namespace wavy {
    frame_generator::frame_generator() {}

    void frame_generator::finalize() {
        m_dir.init();
    }

    size_t frame_generator::add_signal(const std::string& name, const size_t width) {
        auto index = m_dir.add_node(name);
        m_dir.node_data(index).init(width);

        return index;
    }

    void frame_generator::add_transition(const size_t index, const std::string& value, const size_t time) {
        m_max_transition_time = std::max(m_max_transition_time, time);
        m_dir.node_data(index).add_transition(value, time);
    }

    void frame_generator::set_frame(const size_t columns, const size_t rows) {
        m_dir.set_frame(columns, rows - 1);
        m_trace_viewer.set_frame(columns, rows - 1);
    }

    std::string frame_generator::frame() const {
        switch (m_current_view) {
            case view::signal_selection: return m_dir.frame() + "$:>";
            case view::signal_traces: return m_trace_viewer.frame() + "$G>";
            default: return "";
        }
    }

    std::optional<bool> frame_generator::update_frame(const char input) {
        if (input == 'q') {
            return std::nullopt;
        }

        switch (m_current_view) {
            case view::signal_selection: {
                if (input == 0x44) {
                    m_dir.move_up();
                }
                if (input == 0x43) {
                    m_dir.move_down();
                }
                if (input == 'j') {
                    m_dir.move_down();
                }
                if (input == 'k') {
                    m_dir.move_up();
                }
                if (input == 0x20) {
                    m_dir.collapse_toogle();
                }
                if (input == 'b') {
                    m_dir.goto_root();
                }
                if (input == 'a') {
                    for (const auto& index : m_dir.select()) {
                        m_trace_viewer.add_signal(index, m_max_transition_time, m_dir.node_name(index), &m_dir.node_data(index));
                    }
                }
                if (input == 'd') {
                    for (const auto& index : m_dir.unselect()) {
                        m_trace_viewer.remove_signal(index);
                    }
                }
                if (input == '/') {
                    return true;
                }
                if (input == '\t') {
                    m_current_view = view::signal_traces;
                }
                break;
            }
            case view::signal_traces: {
                if (input == 0x44 || input == 'h') {
                    m_trace_viewer.move_left();
                }
                if (input == 0x43 || input == 'l') {
                    m_trace_viewer.move_right();
                }
                if (input == 'j') {
                    m_trace_viewer.move_down();
                }
                if (input == 'k') {
                    m_trace_viewer.move_up();
                }
                if (input == 'J') {
                    m_trace_viewer.shift_down();
                }
                if (input == 'K') {
                    m_trace_viewer.shift_up();
                }
                if (input == '+') {
                    m_trace_viewer.zoom_in();
                }
                if (input == '-') {
                    m_trace_viewer.zoom_out();
                }
                if (input == '\t') {
                    m_current_view = view::signal_selection; 
                }
                break;
            }
            default: break;
        }
        
        return false;
    }

    void frame_generator::execute_command(const std::string& command) {
        m_dir.search_for_node(command);
    }
}
