#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace stdext {
    constexpr char DB_SIGNATURE[] = "stdext::db";
    constexpr auto DB_VERSION     = 1;
    constexpr auto DB_MAX_ENTRIES = 256;
    constexpr auto DB_MAX_NAME    = 256;
    constexpr auto DB_MAX_VALUE   = 4096;
    constexpr auto DB_INVALID     = -1;

    struct db_header {
        char signature[10];
        uint32_t version;
        uint64_t timestamp;
    };

    struct db_entry {
        bool is_active;
        char name[DB_MAX_NAME];
        char value[DB_MAX_VALUE];
    };

    typedef struct database database;

    namespace db {
        database* open(const std::string& filepath);

        void sync(database* db);
        void close(database* db);

        std::vector<std::string> list(database* db);

        const db_entry* find_entry(database* db, const std::string& key);
        db_entry* get_entry(database* db, const std::string& key);

        void update(db_entry* entry, const char* value);
        const char* value(db_entry* entry);

        void set(database* db, const std::string& key, const std::string& value);
        void set(database* db, const std::string& key, const char* value);
        const char* get(database* db, const std::string& key);

        void remove(database* db, const std::string& key);
    }
}
