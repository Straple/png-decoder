add_library(crc_calculator STATIC png-decoder/crc_calculator.cpp)

add_library(png_decoder OBJECT
        png-decoder/png_decoder.cpp
        png-decoder/ihdr.cpp
        png-decoder/bit_reader.cpp
        png-decoder/deflate_wrappers.cpp
        )

target_link_libraries(png_decoder
        crc_calculator
        ${CMAKE_SOURCE_DIR}/libdeflate/liblibdeflate.a)

set(PNG_STATIC png_decoder)
