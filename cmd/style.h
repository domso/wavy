#pragma once

#include "utils.h"

namespace cmd {
    namespace styles {
        namespace colors {
            namespace foreground {
                template<int n>
                struct base {
                    constexpr static const auto prefix = concat(to_char_array("\033["), to_char_array<n>(), to_char_array("m"));
                    constexpr static const auto postfix = to_char_array("\033[0m");
                };

                typedef base<30> black;
                typedef base<31> red;
                typedef base<32> green;
                typedef base<33> yellow;
                typedef base<34> blue;
                typedef base<35> magenta;
                typedef base<36> cyan;
                typedef base<37> white;
                typedef base<90> bright_black;
                typedef base<91> bright_red;
                typedef base<92> bright_green;
                typedef base<93> bright_yellow;
                typedef base<94> bright_blue;
                typedef base<95> bright_magenta;
                typedef base<96> bright_cyan;
                typedef base<97> bright_white;

                template<int n>
                struct extended {
                    constexpr static const auto prefix = concat(to_char_array("\033[38;5;"), to_char_array<n>(), to_char_array("m"));
                    constexpr static const auto postfix = to_char_array("\033[0m");
                };
                
                template<int r, int g, int b>
                struct rgb {
                    constexpr static const auto prefix = concat(to_char_array("\033[38;2;"), to_char_array<r>(), to_char_array(";"), to_char_array<g>(), to_char_array(";"), to_char_array<b>(), to_char_array("m"));
                    constexpr static const auto postfix = to_char_array("\033[0m");
                };
            }
            namespace background {
                template<int n>
                struct base {
                    constexpr static const auto prefix = concat(to_char_array("\033["), to_char_array<n>(), to_char_array("m"));
                    constexpr static const auto postfix = to_char_array("\033[0m");
                };

                typedef base<40> black;
                typedef base<41> red;
                typedef base<42> green;
                typedef base<43> yellow;
                typedef base<44> blue;
                typedef base<45> magenta;
                typedef base<46> cyan;
                typedef base<47> white;
                typedef base<100> bright_black;
                typedef base<101> bright_red;
                typedef base<102> bright_green;
                typedef base<103> bright_yellow;
                typedef base<104> bright_blue;
                typedef base<105> bright_magenta;
                typedef base<106> bright_cyan;
                typedef base<107> bright_white;

                template<int n>
                struct extended {
                    constexpr static const auto prefix = concat(to_char_array("\033[48;5;"), to_char_array<n>(), to_char_array("m"));
                    constexpr static const auto postfix = to_char_array("\033[0m");
                };
                
                template<int r, int g, int b>
                struct rgb {
                    constexpr static const auto prefix = concat(to_char_array("\033[48;2;"), to_char_array<r>(), to_char_array(";"), to_char_array<g>(), to_char_array(";"), to_char_array<b>(), to_char_array("m"));
                    constexpr static const auto postfix = to_char_array("\033[0m");
                };
            }
        }
        namespace font {
            template<int n>
            struct base {
                constexpr static const auto prefix = concat(to_char_array("\033["), to_char_array<n>(), to_char_array("m"));
                constexpr static const auto postfix = to_char_array("\033[0m");
            };

            typedef base<1> bold;
            typedef base<2> faint;
            typedef base<3> italic;
            typedef base<4> underline;
            typedef base<5> slow_blink;
            typedef base<6> rapid_blink;
            typedef base<7> invert;
            typedef base<8> hide;
            typedef base<9> crossed_out;
        }
        template<typename... Ts>
        struct group {
            constexpr static const auto prefix = concat(Ts::prefix...);
            constexpr static const auto postfix = concat(Ts::postfix...);
        };
    }
    template<typename... Ts>
    constexpr std::string add_style(const std::string& s) {
        auto prefix = concat_zero(Ts::prefix...);
        auto postfix = concat_zero(Ts::postfix...);
        
        return std::string(prefix.data()) + s + std::string(postfix.data());
    }
    template<typename... Ts>
    constexpr std::string begin_style() {
        auto prefix = concat_zero(Ts::prefix...);
        auto postfix = concat_zero(Ts::postfix...);
        
        return std::string(prefix.data());
    }
    template<typename... Ts>
    constexpr std::string end_style() {
        auto postfix = concat_zero(Ts::postfix...);
        
        return std::string(postfix.data());
    }
}
