#pragma once

#include <cstdint>

namespace crc_calculator {
    void add_bytes(char *buffer, std::size_t byte_count);
    void reset();
    uint32_t get_checksum();
};// namespace crc_calculator
