#pragma once

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

namespace cmd {
    namespace window {
        inline std::pair<int, int> size() {
            struct winsize ws;
            ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

            return {ws.ws_col, ws.ws_row};
        }
        namespace cursor {
            inline void up(const int n) {
                std::cout << "\033[" + std::to_string(n) + "A";
            }
            inline void down(const int n) {
                std::cout << "\033[" + std::to_string(n) + "B";
            }
            inline void right(const int n) {
                std::cout << "\033[" + std::to_string(n) + "C";
            }
            inline void left(const int n) {
                std::cout << "\033[" + std::to_string(n) + "D";
            }
            inline void hide() {
                std::cout << "\033[?25l";
            }
            inline void show() {
                std::cout << "\033[?25h";
            }
        }

        inline void clear() {
            auto [width, height] = size();
            for (int row = 0; row < height; row++) {
                std::cout << std::endl;
            }
            cursor::up(height - 1);
        }

        struct terminal_settings {
            struct termios native_handle;
            void enable_canonical() {
                native_handle.c_lflag |= ICANON;
            }
            void disable_canonical() {
                native_handle.c_lflag &= ~ICANON;
            }
            void enable_echo() {
                native_handle.c_lflag |= ECHO;
            }
            void disable_echo() {
                native_handle.c_lflag &= ~ECHO;
            }
        };

        inline terminal_settings read_terminal_settings() {
            terminal_settings settings;
            tcgetattr(STDIN_FILENO, &(settings.native_handle));
            return settings;
        }

        inline void write_terminal_settings(const terminal_settings settings) {
            tcsetattr(STDIN_FILENO, TCSANOW, &(settings.native_handle));
        }
    }
}
