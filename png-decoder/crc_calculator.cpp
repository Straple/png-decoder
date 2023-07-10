#include "crc_calculator.hpp"
#include "boost/crc.hpp"

namespace crc_calculator {
    boost::crc_32_type crc_accumulate;

    void add_bytes(char *buffer, std::size_t byte_count) {
        crc_accumulate.process_bytes(buffer, byte_count);
    }
    void reset() {
        crc_accumulate.reset();
    }
    uint32_t get_checksum() {
        return crc_accumulate.checksum();
    }
}// namespace crc_calculator