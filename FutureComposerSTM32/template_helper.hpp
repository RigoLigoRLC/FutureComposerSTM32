
#pragma once

#include <cstdlib>
#include <algorithm>

// https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/
// For passing string literal in template arguments
template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    
    char value[N];
    static const auto size = N;
};

template<StringLiteral Data, size_t Offset>
constexpr uint32_t ExtractU32BE() {
    return uint32_t(Data.value[Offset + 0]) << 24 |
           uint32_t(Data.value[Offset + 1]) << 16 |
           uint32_t(Data.value[Offset + 2]) << 8 |
           uint32_t(Data.value[Offset + 3]);
}
