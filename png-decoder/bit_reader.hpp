#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

struct BitReaderException : std::runtime_error {
    explicit BitReaderException(const std::string &message);
};

class BitReader {
    uint8_t *data;
    int bits_accum = 7;

public:
    BitReader(uint8_t *data_);

    uint32_t read(int bits_count);
};