#pragma once

#include <catch.hpp>

#include <cmath>
#include <iostream>
#include <optional>
#include <string>

#include "png-decoder/image.hpp"
#include "png-decoder/libpng_wrappers.hpp"
#include "png-decoder/png_decoder.hpp"

#ifndef TASK_DIR
#define TASK_DIR "."
#endif

inline std::string ConstructBasePath() {
    std::string result(TASK_DIR);
    return result;
}

const std::string kBasePath = ConstructBasePath();

void Compare(const Image &actual, const Image &expected) {
    REQUIRE(actual.Width() == expected.Width());
    REQUIRE(actual.Height() == expected.Height());
    for (int y = 0; y < actual.Height(); ++y) {
        for (int x = 0; x < actual.Width(); ++x) {
            const auto &actual_data = actual(y, x);
            const auto &expected_data = expected(y, x);
            REQUIRE(actual_data == expected_data);
        }
    }
}

void CheckImage(
        const std::string &filename,
        std::optional<std::string> output_filename = std::nullopt) {
    std::cerr << "Running " << filename << "\n";
    auto image = ReadPng(kBasePath + "tests/" + filename);
    if (output_filename.has_value()) {
        libpng::WriteImage(image, output_filename.value());
    }
    auto ok_image = libpng::ReadImage(kBasePath + "tests/" + filename);
    Compare(image, ok_image);
}
