#include "bit_reader.hpp"

BitReaderException::BitReaderException(const std::string &message)
    : std::runtime_error("BitReaderException: \"" + message + "\"") {
}

BitReader::BitReader(uint8_t *data_) : data(data_) {
}

uint32_t BitReader::read(int bits_count) {
    if (bits_count > 32) {
        throw BitReaderException("call read(), bits_count = " + std::to_string(bits_count) + " more than 32");
    }
    uint32_t val = 0;
    for (int bit = 0; bit < bits_count; bit++) {
        val <<= 1;
        val |= ((*data >> bits_accum) & 1);
        bits_accum--;
        if (bits_accum < 0) {
            bits_accum = 7;
            data++;
        }
    }
    return val;
}
