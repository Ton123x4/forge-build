#include "stdext/crypto.hpp"
#include <array>
#include <random>

namespace stdext::crypto {
    namespace hash {
        constexpr auto CRC32_TABLE = ([] {
            auto table = std::array<uint32_t, 256>();

            for (uint32_t i = 0; i < 256; i++) {
                auto crc = i;

                for (auto bit = 0; bit < 8; bit++) {
                    if (crc & 1) {
                        crc = (crc >> 1) ^ 0xEDB88320u;
                    } else {
                        crc >>= 1;
                    }
                }

                table[i] = crc;
            }

            return table;
        })();


        crc32_context::crc32_context()
            : _state(0xFFFFFFFFu) {}

        void crc32_context::update(const char* data, size_t size) {
            for (size_t i = 0; i < size; i++) {
                auto index = (_state ^ static_cast<std::uint8_t>(data[i])) & 0xFF;

                _state = CRC32_TABLE[index] ^ (_state >> 8);
            }
        }

        std::uint32_t crc32_context::finalize() const {
            return ~_state;
        }


        fnv_context::fnv_context()
            : _state(0xcbf29ce484222325ULL) {}

        void fnv_context::update(const char* data, size_t size) {
            for (size_t i = 0; i < size; i++) {
                _state ^= static_cast<uint8_t>(data[i]);
                _state *= 0x100000001b3ULL;
            }
        }

        std::uint64_t fnv_context::finalize() const {
            return _state;
        }


        std::uint32_t crc32(const char* data, size_t size) {
            auto context = crc32_context();

            context.update(data, size);
            return context.finalize();
        }

        std::uint64_t fnv(const char* data, size_t size) {
            auto context = fnv_context();

            context.update(data, size);
            return context.finalize();
        }
    }

    constexpr std::string_view UUID_TEMPLATE = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    constexpr std::string_view UUID_CHARACTERS = "0123456789abcdef";

    constexpr char STRING_CHARACTERS[] = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";
    constexpr auto STRING_CHARACTERS_LENGTH = sizeof(STRING_CHARACTERS);

    std::mt19937& random_engine() {
        static thread_local auto engine = std::mt19937(std::random_device{}());
        return engine;
    }

    std::string uuid() {
        auto result = std::string();

        result.reserve(UUID_TEMPLATE.length());

        auto& engine = random_engine();
        auto distribution = std::uniform_int_distribution<int>(0, 15);

        for (const char character : UUID_TEMPLATE) {
            switch (character) {
                case 'x':
                    result.push_back(UUID_CHARACTERS[distribution(engine)]);
                    break;

                case 'y':
                    result.push_back(UUID_CHARACTERS[(distribution(engine) & 0x3) | 0x8]);
                    break;

                default:
                    result.push_back(character);
                    break;
            }
        }

        return result;
    }

    std::vector<std::uint8_t> random_bytes(size_t size) {
        auto bytes = std::vector<uint8_t>(size);
        auto distribution = std::uniform_int_distribution<int>(0, 255);
        auto engine = random_engine();

        for (auto& byte : bytes) {
            byte = static_cast<uint8_t>(distribution(engine));
        }

        return bytes;
    }

    std::string random_string(size_t length) {
        auto result = std::string();
        auto engine = random_engine();
        auto distribution = std::uniform_int_distribution<size_t>(STRING_CHARACTERS_LENGTH - 2);

        for (auto i = 0ull; i < length; i++) {
            result.push_back(STRING_CHARACTERS[distribution(engine)]);
        }

        return result;
    }
}
