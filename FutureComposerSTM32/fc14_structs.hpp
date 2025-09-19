
#pragma once

#include "template_helper.hpp"

namespace fc14 {

template <StringLiteral Data>
struct Header {
    Header()
        : patternIndexTableSize     (ExtractU32BE<Data, 0x4>())
        , patternDataOffset         (ExtractU32BE<Data, 0x8>())
        , patternDataSize           (ExtractU32BE<Data, 0xC>())
        , frequencySequenceOffset   (ExtractU32BE<Data, 0x10>())
        , frequencySequenceSize     (ExtractU32BE<Data, 0x14>())
        , volumeSequenceOffset      (ExtractU32BE<Data, 0x18>())
        , volumeSequenceSize        (ExtractU32BE<Data, 0x1C>())
        , sampleDataOffset          (ExtractU32BE<Data, 0x20>())
        , waveformDataOffset        (ExtractU32BE<Data, 0x24>())
    {}

    uint32_t patternIndexTableSize;
    uint32_t patternDataOffset;
    uint32_t patternDataSize;
    uint32_t frequencySequenceOffset;
    uint32_t frequencySequenceSize;
    uint32_t volumeSequenceOffset;
    uint32_t volumeSequenceSize;
    uint32_t sampleDataOffset;
    uint32_t waveformDataOffset;
};



}
