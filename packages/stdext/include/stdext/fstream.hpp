#pragma once

#include "filesystem.hpp"
#include <cstdio>

namespace stdext::file {
    FILE* open(const fs::path& filename, const char* mode);
    void close(FILE* stream);

    size_t read(FILE* stream, char* buffer, size_t size);
    size_t write(FILE* stream, const char* data, size_t size);

    size_t read(FILE* stream, char* buffer, size_t size, size_t entries);
    size_t write(FILE* stream, const char* data, size_t size, size_t entries);

    size_t get_cursor(FILE* stream);
    size_t set_cursor(FILE* stream, size_t pos, int start);
    size_t rewind(FILE* stream);

    size_t size(FILE* stream);
    bool flush(FILE* stream);
    bool eof(FILE* stream);

    bool error(FILE* stream);
    void clear_error(FILE* stream);
}
