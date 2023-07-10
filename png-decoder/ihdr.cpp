#include "ihdr.hpp"
#include <cstring>

IHDRException::IHDRException(const std::string &message)
    : std::runtime_error("IHDRException: \"" + message + "\"") {
}

void IHDR::read(std::string data) {
    if (data.size() != sizeof(IHDR)) {
        throw IHDRException("bad read data");
    }

    std::memcpy(reinterpret_cast<char *>(this), data.data(), sizeof(IHDR));
    std::reverse(reinterpret_cast<char *>(&width), reinterpret_cast<char *>(&width) + sizeof(uint32_t));
    std::reverse(reinterpret_cast<char *>(&height), reinterpret_cast<char *>(&height) + sizeof(uint32_t));

    if (width == 0) {
        throw IHDRException("invalid zero width");
    }
    if (height == 0) {
        throw IHDRException("invalid zero height");
    }

    uint32_t max_len = static_cast<uint32_t>(1) << 31;
    if (width > max_len) {
        throw IHDRException("width = " + std::to_string(width) + ", more than 2^31");
    }
    if (height > max_len) {
        throw IHDRException("height = " + std::to_string(height) + ", more than 2^31");
    }

    if (bit_depth == 0 || (bit_depth & (bit_depth - 1)) != 0 || bit_depth > 16) {
        throw IHDRException("invalid bit_depth = " + std::to_string(bit_depth) + ", != 1, 2, 4, 8 or 16");
    }
    if (color_type != 0 && color_type != 2 && color_type != 3 && color_type != 4 && color_type != 6) {
        throw IHDRException("invalid color_type = " + std::to_string(color_type) + ", != 0, 2, 3, 4 or 6");
    }
    if (compression_method != 0) {
        throw IHDRException("invalid compression_method = " + std::to_string(compression_method) + ", != 0");
    }
    if (filter_method != 0) {
        throw IHDRException("invalid filter_method = " + std::to_string(filter_method) + ", != 0");
    }
    if (interlace_method != 0 && interlace_method != 1) {
        throw IHDRException("invalid interlace_method = " + std::to_string(interlace_method) + ", != 0 or 1");
    }
}

std::size_t IHDR::get_pixel_len_in_bits() {
    if (color_type == 0) {
        return bit_depth;
    } else if (color_type == 2) {
        return 3 * bit_depth;
    } else if (color_type == 3) {
        return bit_depth;
    } else if (color_type == 4) {
        return 2 * bit_depth;
    } else if (color_type == 6) {
        return 4 * bit_depth;
    }
    throw IHDRException("call get_pixel_len_in_bits(), why color_type = " + std::to_string(color_type) + ", != 0, 2, 3, 4 or 6?");
}
