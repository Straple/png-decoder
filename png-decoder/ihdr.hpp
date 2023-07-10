#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

struct IHDRException : std::runtime_error {
    explicit IHDRException(const std::string &message);
};

#pragma pack(push, 1)

struct IHDR {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;

    void read(std::string data);

    std::size_t get_pixel_len_in_bits();
};

#pragma pack(pop)
