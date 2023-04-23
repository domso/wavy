#pragma once

#include <tuple>

namespace cmd {
    template<int n>
    consteval int const_int2str_length() {
        char c = n % 10;
        int next = n / 10;
        if (next > 0) {
            return const_int2str_length<n / 10>() + 1;
        } else {
            return 1;
        }
    }

    template<int n, int size>
    consteval std::array<char, size> to_char_array_internal() {
        std::array<char, size> result;
        auto current = n;
        for (auto i = 0; i < size; i++) {
            result[size - 1 - i] = 48 + current % 10;
            current = current / 10;
        }

        return result;
    }

    template<int n>
    consteval std::array<char, const_int2str_length<n>()> to_char_array() {
        return to_char_array_internal<n, const_int2str_length<n>()>();
    }

    template<int n>
    consteval std::array<char, n - 1> to_char_array(const char (&c)[n]) {
        std::array<char, n - 1> result;
        
        for (int i = 0; i < n - 1; i++) {
            result[i] = c[i];
        }

        return result;
    }

    template <std::size_t n0>
    consteval auto concat_internal(const std::array<char, n0>& array0)
    {
        return array0;
    }

    template <std::size_t n0, std::size_t n1, std::size_t... ns>
    consteval auto concat_internal(const std::array<char, n0>& array0, const std::array<char, n1>& array1, const std::array<char, ns>&... arrays)
    {
        std::array<char, (n0 + n1)> buffer;

        for (int i = 0; i < n0; i++) {
            buffer[i] = array0[i];
        }
        for (int i = 0; i < n1; i++) {
            buffer[n0 + i] = array1[i];
        }
         
        if constexpr (std::tuple_size<decltype(std::make_tuple(&arrays...))>() == 0) {        
            return buffer;
        } else {
            return concat_internal<n0 + n1, ns...>(buffer, arrays...);
        }
    }

    template <std::size_t... ns>
    consteval auto concat(const std::array<char, ns>&... arrays)
    {
        return concat_internal(arrays...);
    }

    template <std::size_t... ns>
    consteval auto concat_zero(const std::array<char, ns>&... arrays)
    {
        std::array<char, 1UL> zero_ending = {'\0'};
        return concat_internal(arrays..., zero_ending);
    }
}
