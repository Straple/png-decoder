#include "png_decoder.hpp"
#include "bit_reader.hpp"
#include "crc_calculator.hpp"
#include "deflate_wrappers.hpp"
#include <fstream>

const uint8_t PNG_SIGNATURE[] = {137, 80, 78, 71, 13, 10, 26, 10};

//==============//
//==EXCEPTIONS==//
//==============//

PNGDecoderException::PNGDecoderException(const std::string &message)
    : std::runtime_error("PNGDecoderException: \"" + message + "\"") {
}

FailedToReadException::FailedToReadException(const std::string &message)
    : PNGDecoderException("\nfailed to read: " + message) {
}

InvalidPNGFormatException::InvalidPNGFormatException(const std::string &message)
    : PNGDecoderException("\ninvalid PNG format: " + message) {
}

//==================//
//==REMOVE FILTERS==//
//==================//

int PaethPredictor(int a, int b, int c) {
    int p = a + b - c;
    int pa = std::abs(p - a);
    int pb = std::abs(p - b);
    int pc = std::abs(p - c);
    if (pa <= pb && pa <= pc) {
        return a;
    } else if (pb <= pc) {
        return b;
    } else {
        return c;
    }
}

void remove_sub_filter(uint8_t *data, std::size_t byte_count, std::size_t bpp) {
    for (std::size_t byte = 0; byte < byte_count; byte++) {
        if (byte >= bpp) {
            data[byte] += data[byte - bpp];
        }
    }
}

void remove_up_filter(uint8_t *data, std::size_t byte_count) {
    for (std::size_t byte = 0; byte < byte_count; byte++) {
        data[byte] += data[byte - (byte_count + 1)];
    }
}

void remove_average_filter(uint8_t *data, std::size_t byte_count, std::size_t bpp, std::size_t row) {
    for (std::size_t byte = 0; byte < byte_count; byte++) {
        int left = 0;
        if (byte >= bpp) {
            left = data[byte - bpp];
        }
        int top = 0;
        if (row != 0) {
            top = data[byte - (byte_count + 1)];
        }
        data[byte] += (top + left) / 2;
    }
}

void remove_paeth_filter(uint8_t *data, std::size_t byte_count, std::size_t bpp, std::size_t row) {
    for (std::size_t byte = 0; byte < byte_count; byte++) {
        int left = 0;
        if (byte >= bpp) {
            left = data[byte - bpp];
        }
        int top = 0;
        if (row > 0) {
            top = data[byte - (byte_count + 1)];
        }
        int top_left = 0;
        if (byte >= bpp && row > 0) {
            top_left = data[byte - (byte_count + 1) - bpp];
        }
        data[byte] += PaethPredictor(left, top, top_left);
    }
}

void remove_filters(uint8_t *data, std::size_t pixel_len_in_bits, std::size_t row_len_in_bytes, std::size_t height) {
    std::size_t bpp = (pixel_len_in_bits + 7) / 8;

    for (std::size_t row = 0; row < height; row++, data += row_len_in_bytes + 1) {
        if (*data == 0) {
            // нет фильтров
        } else if (*data == 1) {
            remove_sub_filter(data + 1, row_len_in_bytes, bpp);
        } else if (*data == 2) {
            if (row != 0) {
                remove_up_filter(data + 1, row_len_in_bytes);
            }
        } else if (*data == 3) {
            remove_average_filter(data + 1, row_len_in_bytes, bpp, row);
        } else if (*data == 4) {
            remove_paeth_filter(data + 1, row_len_in_bytes, bpp, row);
        } else {
            throw InvalidPNGFormatException("invalid row filter mode = " + std::to_string(*data) + ", != 0-4");
        }
    }
}

//===========//
//==READING==//
//===========//

enum class read_context_t {
    PNG_SIGNATURE,
    CHUNK_DATA_LENGTH,
    CHUNK_TYPE_CODE,
    CHUNK_DATA,
    CHUNK_CRC
};

void read_bytes(std::istream &input, void *buffer, int byte_count, read_context_t type, bool need_to_reverse_bytes) {
    char *buffer_char = reinterpret_cast<char *>(buffer);
    try {
        input.read(buffer_char, byte_count);
        if (need_to_reverse_bytes) {
            std::reverse(buffer_char, buffer_char + byte_count);
        }
    } catch (std::exception &error) {
        std::string context;
        if (type == read_context_t::PNG_SIGNATURE) {
            context = "png signature";
        } else if (type == read_context_t::CHUNK_DATA_LENGTH) {
            context = "chunk data length";
        } else if (type == read_context_t::CHUNK_TYPE_CODE) {
            context = "chunk type code";
        } else if (type == read_context_t::CHUNK_DATA) {
            context = "chunk data";
        } else if (type == read_context_t::CHUNK_CRC) {
            context = "chunk CRC";
        }

        throw FailedToReadException("\"" + context + "\"\ncaught message: " + error.what());
    }
}

void read_signature(std::istream &input) {
    char signature[8];
    read_bytes(input, signature, 8, read_context_t::PNG_SIGNATURE, false);

    if (std::memcmp(PNG_SIGNATURE, signature, 8) != 0) {
        std::string actual_values;
        for (std::size_t byte = 0; byte < 8; byte++) {
            actual_values += std::to_string(static_cast<uint8_t>(signature[byte]));
            if (byte != 7) {
                actual_values += ' ';
            }
        }
        throw InvalidPNGFormatException("invalid signature: " + actual_values);
    }
}

std::string read_chunk(std::istream &input, char chunk_type_code[4]) {
    uint32_t data_length;
    read_bytes(input, &data_length, 4, read_context_t::CHUNK_DATA_LENGTH, true);

    if (data_length > (static_cast<uint32_t>(1) << 31)) {
        throw InvalidPNGFormatException("invalid chunk data length: " + std::to_string(data_length) + ", more than 2^31");
    }

    read_bytes(input, chunk_type_code, 4, read_context_t::CHUNK_TYPE_CODE, false);

    std::string data(data_length, '\0');

    read_bytes(input, data.data(), data_length, read_context_t::CHUNK_DATA, false);

    uint32_t actual_crc;
    read_bytes(input, &actual_crc, 4, read_context_t::CHUNK_CRC, true);

    crc_calculator::reset();
    crc_calculator::add_bytes(chunk_type_code, 4);
    crc_calculator::add_bytes(data.data(), data_length);
    uint32_t correct_crc = crc_calculator::get_checksum();

    if (actual_crc != correct_crc) {
        throw InvalidPNGFormatException("\ninvalid CRC: actual = " + std::to_string(actual_crc) +
                                        ", correct = " + std::to_string(correct_crc));
    }

    return data;
}

RGB read_pixel(IHDR ihdr, BitReader &bit_reader, const std::string &palette) {
    RGB result;
    {
        std::size_t alpha_channel_bit_depth = ihdr.bit_depth;
        if (ihdr.color_type == 3) {// PLTE
            alpha_channel_bit_depth = 8;
        }
        result.a = (1 << alpha_channel_bit_depth) - 1;
    }

    if (ihdr.color_type == 0) {
        result.r = result.g = result.b = bit_reader.read(ihdr.bit_depth);
    } else if (ihdr.color_type == 2) {
        result.r = bit_reader.read(ihdr.bit_depth);
        result.g = bit_reader.read(ihdr.bit_depth);
        result.b = bit_reader.read(ihdr.bit_depth);
    } else if (ihdr.color_type == 3) {
        std::size_t color_index = bit_reader.read(ihdr.bit_depth);
        color_index *= 3;
        if (color_index + 2 >= palette.size()) {
            throw InvalidPNGFormatException("pixel index more than palette size");
        }
        result.r = static_cast<uint8_t>(palette[color_index]);
        result.g = static_cast<uint8_t>(palette[color_index + 1]);
        result.b = static_cast<uint8_t>(palette[color_index + 2]);
    } else if (ihdr.color_type == 4) {
        result.r = result.g = result.b = bit_reader.read(ihdr.bit_depth);
        result.a = bit_reader.read(ihdr.bit_depth);
    } else if (ihdr.color_type == 6) {
        result.r = bit_reader.read(ihdr.bit_depth);
        result.g = bit_reader.read(ihdr.bit_depth);
        result.b = bit_reader.read(ihdr.bit_depth);
        result.a = bit_reader.read(ihdr.bit_depth);
    } else {
        throw PNGDecoderException("call read_pixel(), invalid ihdr.color_type = " +
                                  std::to_string(ihdr.color_type) + ", != 0, 2, 3, 4 or 6");
    }
    return result;
}

//===============//
//==UNINTERLACE==//
//===============//

// return (height, width)
std::pair<std::size_t, std::size_t> get_subimage_shape_in_interlace(int pass_cnt, std::size_t image_height, std::size_t image_width) {
    switch (pass_cnt) {
        case 0:
            return {image_height, image_width};
        case 1:
            return {(image_height + 7) / 8, (image_width + 7) / 8};
        case 2:
            return {(image_height + 7) / 8, (image_width - 4 + 7) / 8};
        case 3:
            return {(image_height - 4 + 7) / 8, (image_width + 3) / 4};
        case 4:
            return {(image_height + 3) / 4, (image_width - 2 + 3) / 4};
        case 5:
            return {(image_height - 2 + 3) / 4, (image_width + 1) / 2};
        case 6:
            return {(image_height + 1) / 2, (image_width - 1 + 1) / 2};
        case 7:
            return {(image_height - 1 + 1) / 2, image_width};
    }
    throw PNGDecoderException("call get_subimage_shape_in_interlace(), invalid pass_cnt = " +
                              std::to_string(pass_cnt) + ", != 0-7");
}

Image uninterlace(IHDR ihdr, const std::string &palette, int pass_cnt, uint8_t *data) {
    auto [height, width] = get_subimage_shape_in_interlace(pass_cnt, ihdr.height, ihdr.width);

    std::size_t pixel_len_in_bits = ihdr.get_pixel_len_in_bits();
    std::size_t row_len_in_bytes = (pixel_len_in_bits * width + 7) / 8;

    remove_filters(data, pixel_len_in_bits, row_len_in_bytes, height);

    BitReader bit_reader(data);

    Image result(height, width);

    for (std::size_t row = 0; row < height; row++) {
        bit_reader.read(8);// skip byte filter type
        for (std::size_t column = 0; column < width; column++) {
            result(row, column) = read_pixel(ihdr, bit_reader, palette);
        }
        bit_reader.read((8 - (pixel_len_in_bits * width) % 8) % 8);// skip to byte
    }
    return result;
}

//===============//
//==PNG DECODER==//
//===============//

PNGDecoder::PNGDecoder(std::istream &input) {
    read_signature(input);

    std::string data_accum;

    bool is_read_ihdr = false;
    bool is_read_palette = false;
    while (true) {
        char chunk_type_code[4];
        auto chunk_data = read_chunk(input, chunk_type_code);
        if (memcmp(chunk_type_code, "IHDR", 4) == 0) {
            is_read_ihdr = true;
            ihdr.read(chunk_data);
        } else if (memcmp(chunk_type_code, "IDAT", 4) == 0) {
            data_accum += chunk_data;
        } else if (memcmp(chunk_type_code, "PLTE", 4) == 0) {
            is_read_palette = true;
            palette += chunk_data;
        } else if (memcmp(chunk_type_code, "IEND", 4) == 0) {
            break;
        }
    }

    if (!is_read_ihdr) {
        throw InvalidPNGFormatException("missing chunk \"IHDR\"");
    }

    if (!is_read_palette && ihdr.color_type == 3) {
        throw InvalidPNGFormatException("missing chunk \"PLTE\", but palette is used");
    }

    DeflateWrapper deflate_wrapper;
    pixels_data = deflate_wrapper.deflate(data_accum);
}

Image PNGDecoder::build_image() {
    Image result(ihdr.height, ihdr.width);

    auto set_subimage = [&](const Image &subimage,
                            std::size_t start_row, std::size_t start_column,
                            std::size_t step_row, std::size_t step_column) {
        for (uint32_t row = start_row, uninterlace_row = 0; row < ihdr.height;
             row += step_row, uninterlace_row++) {
            for (uint32_t column = start_column, uninterlace_column = 0; column < ihdr.width;
                 column += step_column, uninterlace_column++) {
                result(row, column) = subimage(uninterlace_row, uninterlace_column);
            }
        }
    };

    // (pass_cnt, start_row, start_column, step_row, step_column)
    std::vector<std::vector<int>> image_build_args;
    if (ihdr.interlace_method == 0) {
        image_build_args = {
                {0, 0, 0, 1, 1},
        };
    } else {
        image_build_args = {
                {1, 0, 0, 8, 8},
                {2, 0, 4, 8, 8},
                {3, 4, 0, 8, 4},
                {4, 0, 2, 4, 4},
                {5, 2, 0, 4, 2},
                {6, 0, 1, 2, 2},
                {7, 1, 0, 2, 1},
        };
    }

    uint8_t *data = reinterpret_cast<uint8_t *>(pixels_data.data());
    std::size_t pixels_data_avail = pixels_data.size();

    for (auto vals: image_build_args) {
        auto [height, width] = get_subimage_shape_in_interlace(vals[0], ihdr.height, ihdr.width);

        std::size_t pixel_len_in_bits = ihdr.get_pixel_len_in_bits();
        std::size_t row_len_in_bytes = (pixel_len_in_bits * width + 7) / 8;
        std::size_t subimage_size = (row_len_in_bytes + 1) * height;

        if (pixels_data_avail < subimage_size) {
            throw InvalidPNGFormatException("short pixel data length");
        }
        pixels_data_avail -= subimage_size;

        set_subimage(uninterlace(ihdr, palette, vals[0], data), vals[1], vals[2], vals[3], vals[4]);

        data += subimage_size;
    }

    if (pixels_data_avail != 0) {
        throw InvalidPNGFormatException("too much length pixel data");
    }

    if (ihdr.bit_depth != 8 && ihdr.color_type != 3) {
        auto cast_to_8_bits = [&](int val) {
            return val * 0xff / ((1 << ihdr.bit_depth) - 1);
        };
        for (int row = 0; row < result.Height(); row++) {
            for (int column = 0; column < result.Width(); column++) {
                result(row, column).r = cast_to_8_bits(result(row, column).r);
                result(row, column).g = cast_to_8_bits(result(row, column).g);
                result(row, column).b = cast_to_8_bits(result(row, column).b);
                result(row, column).a = cast_to_8_bits(result(row, column).a);
            }
        }
    }
    return result;
}

Image ReadPng(std::string_view filename) {
    std::ifstream file_input(filename.data(), std::ios_base::in | std::ios_base::binary);
    // сначала нужно проверить, что файл открыт
    if (!file_input.is_open()) {
        throw PNGDecoderException("Unable to open file \"" + std::string(filename) + "\"");
    }
    // а уже потом ставить обработку исключений, иначе упадет
    file_input.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    return PNGDecoder(file_input).build_image();
}
