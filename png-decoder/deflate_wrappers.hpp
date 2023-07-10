#pragma once

#include "../libdeflate/libdeflate.h"
#include <stdexcept>
#include <string>

struct DeflateWrapperException : std::runtime_error {
    explicit DeflateWrapperException(const std::string &message);
};

class DeflateWrapper {
    libdeflate_decompressor *decompressor = nullptr;

public:
    DeflateWrapper();

    ~DeflateWrapper();

    std::string deflate(std::string data);
};