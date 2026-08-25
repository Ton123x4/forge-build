#include "stdext/fstream.hpp"

namespace stdext::file {
    FILE* open(const fs::path& filename, const char* mode) {
        return std::fopen(filename.string().c_str(), mode);
    }

    void close(FILE* stream) {
        std::fclose(stream);
    }

    size_t read(FILE* stream, char* buffer, size_t size) {
        return std::fread(buffer, 1, size, stream);
    }

    size_t write(FILE* stream, const char* data, size_t size) {
        return std::fwrite(data, 1, size, stream);
    }

    size_t read(FILE* stream, char* buffer, size_t size, size_t entries) {
        return std::fread(buffer, size, entries, stream);
    }

    size_t write(FILE* stream, const char* data, size_t size, size_t entries) {
        return std::fwrite(data, size, entries, stream);
    }

    size_t get_cursor(FILE* stream) {
        return (size_t)std::ftell(stream);
    }

    size_t set_cursor(FILE* stream, size_t pos, int start) {
        std::fseek(stream, (long)pos, start);
        return (size_t)std::ftell(stream);
    }

    size_t rewind(FILE* stream) {
        std::fseek(stream, 0, SEEK_SET);
        return 0;
    }

    size_t size(FILE* stream) {
        auto current = std::ftell(stream);
        std::fseek(stream, 0, SEEK_END);

        auto end = std::ftell(stream);
        std::fseek(stream, current, SEEK_SET);

        return end;
    }

    bool flush(FILE* stream) {
        return std::fflush(stream) == 0;
    }

    bool eof(FILE* stream) {
        return std::feof(stream) != 0;
    }

    bool error(FILE* stream) {
        return std::ferror(stream) != 0;
    }

    void clear_error(FILE* stream) {
        std::clearerr(stream);
    }
}
