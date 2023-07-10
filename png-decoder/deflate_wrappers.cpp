#include "deflate_wrappers.hpp"

DeflateWrapperException::DeflateWrapperException(const std::string &message)
    : std::runtime_error("DeflateWrapperException: \"" + message + "\"") {
}

DeflateWrapper::DeflateWrapper()
    : decompressor(libdeflate_alloc_decompressor()) {
    if (decompressor == nullptr) {
        throw DeflateWrapperException("bad alloc decompressor");
    }
}

DeflateWrapper::~DeflateWrapper() {
    libdeflate_free_decompressor(decompressor);
}

std::string DeflateWrapper::deflate(std::string data) {
    std::string result(1000 * data.size(), '\0');
    size_t actual_out_nbytes_ret = 0;
    libdeflate_result result_code = libdeflate_zlib_decompress(
            decompressor, data.data(), data.size(), result.data(), result.size(),
            &actual_out_nbytes_ret);

    if (result_code == LIBDEFLATE_SUCCESS) {
        // ok
    } else if (result_code == LIBDEFLATE_BAD_DATA) {
        throw DeflateWrapperException(
                "decompress bad data, see LIBDEFLATE_BAD_DATA");
    } else if (result_code == LIBDEFLATE_SHORT_OUTPUT) {
        throw DeflateWrapperException(
                "decompress short output, see LIBDEFLATE_SHORT_OUTPUT");
    } else if (result_code == LIBDEFLATE_INSUFFICIENT_SPACE) {
        throw DeflateWrapperException(
                "decompress insufficient space, see LIBDEFLATE_INSUFFICIENT_SPACE");
    }
    result.resize(actual_out_nbytes_ret);
    return result;
}
