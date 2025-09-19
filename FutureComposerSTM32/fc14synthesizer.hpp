
#pragma once

#include "template_helper.hpp"
#include "fc14audiodecoder/FC.h"
#include "fc14audiodecoder/LamePaula.h"
#include <algorithm>
#include <array>
#include <cstdint>

template <size_t T>
consteval uint32_t SwapByteorderU32At(std::array<uint8_t, T> &arr, size_t offset) {
    uint8_t byte0 = arr[offset + 0], byte1 = arr[offset + 1], 
            byte2 = arr[offset + 2], byte3 = arr[offset + 3];
    arr[offset + 0] = byte3;
    arr[offset + 1] = byte2;
    arr[offset + 2] = byte0;
    arr[offset + 3] = byte1;
    return uint32_t(byte0) << 24 | uint32_t(byte1) << 16 | uint32_t(byte2) << 8 |
           byte3;
}

template <size_t T>
consteval uint16_t SwapByteorderU16At(std::array<uint8_t, T> &arr, size_t offset) {
    uint8_t byte0 = arr[offset], byte1 = arr[offset + 1];
    arr[offset + 0] = byte1;
    arr[offset + 1] = byte0;
    return uint16_t(byte0) << 8 | byte1;
}

// Amiga is big endian, so does Future Composer files
// Swapping endianness in runtime is too heavy for MCU, we do it in comptime
template <StringLiteral Data>
consteval auto Fc14ByteorderInversion() {
    std::array<uint8_t, Data.size + 8> ret;
    std::copy_n(Data.value, Data.size, ret.begin());

    // Header offsets
    SwapByteorderU32At(ret, 0x4);
    SwapByteorderU32At(ret, 0x8);
    SwapByteorderU32At(ret, 0xC);
    SwapByteorderU32At(ret, 0x10);
    SwapByteorderU32At(ret, 0x14);
    SwapByteorderU32At(ret, 0x18);
    SwapByteorderU32At(ret, 0x1C);
    const size_t sampleOffset = SwapByteorderU32At(ret, 0x20);
    const size_t waveformOffset = SwapByteorderU32At(ret, 0x24);

    // Sample metadata
    size_t samplePointer = sampleOffset;
    for (int i = 0; i < 10; ++i) {
        SwapByteorderU16At(ret, 0x28 + i * 6);
        SwapByteorderU16At(ret, 0x2A + i * 6);
        SwapByteorderU16At(ret, 0x2C + i * 6);
    }

    // Append silence
    uint8_t silence[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE1 };
    std::copy_n(silence, 8, ret.begin() + Data.size);

    return ret;
}

template <size_t T>
class FC14Synthesizer {
public:
    FC14Synthesizer(const std::array<uint8_t, T> &transformedFc14Data)
        : m_transformedFc14(transformedFc14Data) {
        
        decoder.init((void*)(m_transformedFc14.data()), m_transformedFc14.size());
        decoder.setMixer(&mixer);
        mixer.init(28125, 8, 1, 0);
        decoder.restart();
    }

    void synthesize(void* buffer, size_t lenBytes) {
        if (decoder.songEnd) {
            decoder.restart();
        }
        mixer.fillBuffer(buffer, lenBytes, &decoder);
        uint32_t *bufU32 = (uint32_t *)buffer;
        for (int i = 0; i < lenBytes / 4; i++) {
            bufU32[i] ^= 0x80808080; // Flip MSB to make signed 8-bit PCM unsigned
        }
    }

protected:
    const std::array<uint8_t, T> &m_transformedFc14;
    FC decoder;
    LamePaulaMixer mixer;
};

