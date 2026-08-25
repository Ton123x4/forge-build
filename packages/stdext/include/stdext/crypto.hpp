#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace stdext::crypto {
    namespace hash {
        struct crc32_context {
        private:
            std::uint32_t _state;

        public:
            crc32_context();

            void update(const char* data, size_t size);
            std::uint32_t finalize() const;
        };

        struct fnv_context {
        private:
            std::uint64_t _state;

        public:
            fnv_context();

            void update(const char* data, size_t size);
            std::uint64_t finalize() const;
        };

        std::uint32_t crc32(const char* data, size_t size);

        std::uint64_t fnv(const char* data, size_t size);
    }

    std::vector<std::uint8_t> random_bytes(std::size_t size);

    std::string random_string(std::size_t length);

    std::string uuid();
}
