#pragma once

#include "ihdr.hpp"
#include "image.hpp"
#include <cstring>
#include <stdexcept>
#include <string>

struct PNGDecoderException : std::runtime_error {
    explicit PNGDecoderException(const std::string &message);
};

struct FailedToReadException : PNGDecoderException {
    explicit FailedToReadException(const std::string &message);
};

struct InvalidPNGFormatException : PNGDecoderException {
    explicit InvalidPNGFormatException(const std::string &message);
};

class PNGDecoder {
    IHDR ihdr;
    std::string pixels_data;
    std::string palette;

public:
    PNGDecoder(std::istream &input);

    Image build_image();
};

Image ReadPng(std::string_view filename);