#pragma once

#include <optional>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdint.h>

#include "selection/tree_directory_viewer.h"
#include "waveform/signal_viewer.h"

namespace wavy {
    class frame_generator {
    public:
        frame_generator();

        void finalize();

        size_t add_signal(const std::string& name, const size_t width);

        void add_transition(const size_t index, const std::string& value, const size_t time);

        void set_frame(const size_t columns, const size_t rows);

        std::string frame() const;

        std::optional<bool> update_frame(const char input);

        void execute_command(const std::string& command);
    private:
        enum class view {signal_selection, signal_traces};
        view m_current_view = view::signal_selection;

        selection::tree_directory_viewer<waveform::signal<uint64_t>> m_dir;

        waveform::signal_viewer<uint64_t> m_trace_viewer;
        size_t m_max_transition_time = 0;
    };
}
