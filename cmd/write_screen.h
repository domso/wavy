#pragma once

#include "window.h"

namespace cmd {
    template<typename T>
    void write_screen(T& frame_generator) {
        auto old_settings = window::read_terminal_settings();
        auto new_settings = old_settings;
        new_settings.disable_canonical();
        new_settings.disable_echo();

        window::write_terminal_settings(new_settings);
        window::cursor::hide();

        char input = 0;
        while (true) {
            auto [columns, rows] = window::size();
            frame_generator.set_frame(columns, rows);

            if (auto enter_line = frame_generator.update_frame(input)) {
                window::cursor::up(rows);

                if (*enter_line) {
                    std::cout << frame_generator.frame() << std::flush;
                    window::write_terminal_settings(old_settings);
                    window::cursor::show();

                    std::string command;

                    std::cin >> command;
                    frame_generator.execute_command(command);
                    window::cursor::up(1);
                                        
                    window::write_terminal_settings(new_settings);
                    window::cursor::hide();
                } else {
                    std::cout << frame_generator.frame() << '\r' << std::flush;
                }
            } else {
                break;
            }

            input = getchar();
        }

        window::write_terminal_settings(old_settings);
        window::cursor::show();
    }
}
