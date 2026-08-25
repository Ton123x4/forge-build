#include "stdext/database.hpp"
#include "stdext/filesystem.hpp"
#include "stdext/time.hpp"
#include <cstring>
#include <cstdio>
#include <filesystem>

namespace stdext {
    typedef struct database {
        db_header header;
        db_entry* entries = nullptr;
        FILE* stream = nullptr;
    } database;

    static void init_db(database* db) {
        auto current_time = stdext::time::unix_milliseconds();

        db->header.version = DB_VERSION;
        db->header.timestamp = current_time;

        memcpy(db->header.signature, DB_SIGNATURE, 10);
        memset(db->entries, 0, DB_MAX_ENTRIES * sizeof(db_entry));
    }

    static void read_db(database* db) {
        fread(&db->header, sizeof(db_header), 1, db->stream);

        if (memcmp(db->header.signature, DB_SIGNATURE, 10) != 0) {
            fclose(db->stream);
            delete[] db->entries;

            db->stream = nullptr;
            db->entries = nullptr;
            return;
        }

        fread(db->entries, sizeof(db_entry), DB_MAX_ENTRIES, db->stream);
    }

    static bool load_db(database* db, const std::string& filepath, bool should_sync) {
        db->stream = fopen(filepath.c_str(), should_sync ? "r+b" : "w+b");

        if (db->stream == nullptr) {
            return false;
        }

        db->entries = new db_entry[DB_MAX_ENTRIES];

        if (should_sync) {
            read_db(db);
        } else {
            init_db(db);
        }

        return true;
    }

    database* db::open(const std::string& filepath) {
        auto* db = new database();
        bool should_sync = false;

        if (stdext::fs::exists(filepath)) {
            should_sync = true;
        }

        if (!load_db(db, filepath, should_sync)) {
            delete db;
            return nullptr;
        }

        if (should_sync == false) {
            db::sync(db);
        }

        return db;
    }

    void db::sync(database* db) {
        rewind(db->stream);
        fwrite(&db->header, sizeof(db->header), 1, db->stream);
        fwrite(db->entries, sizeof(db_entry), DB_MAX_ENTRIES, db->stream);
        fflush(db->stream);
    }

    void db::close(database* db) {
        if (db != nullptr) {
            sync(db);

            fclose(db->stream);
            delete[] db->entries;

            db->stream = nullptr;
            db->entries = nullptr;
        }

        delete db;
    }

    std::vector<std::string> db::list(database* db) {
        std::vector<std::string> result;

        for (int i = 0; i < DB_MAX_ENTRIES; i++) {
            auto* entry = &db->entries[i];

            if (entry->is_active == false) {
                continue;
            }

            result.push_back(entry->name);
        }

        return result;
    }

    const db_entry* db::find_entry(database* db, const std::string& key) {
        for (int i = 0; i < DB_MAX_ENTRIES; i++) {
            auto* entry = &db->entries[i];

            if (entry->is_active == false) {
                continue;
            }

            if (entry->name == key) {
                return entry;
            }
        }

        return nullptr;
    }

    db_entry* db::get_entry(database* db, const std::string& key) {
        db_entry* free_entry = nullptr;

        for (int i = 0; i < DB_MAX_ENTRIES; i++) {
            auto* entry = &db->entries[i];

            if (entry->is_active == false) {
                if (free_entry == nullptr) {
                    free_entry = entry;
                }

                continue;
            }

            if (entry->name == key) {
                return entry;
            }
        }

        if (free_entry != nullptr) {
            strcpy(free_entry->name, key.c_str());
            memset(free_entry->value, 0, DB_MAX_VALUE);
            free_entry->is_active = true;
        }

        return free_entry;
    }

    void db::update(db_entry* entry, const char* value) {
        int length = strlen(value);

        if (length > DB_MAX_VALUE) {
            length += (DB_MAX_VALUE - length);
        }

        memset(entry->value, 0, DB_MAX_VALUE);
        memcpy(entry->value, value, length);
    }

    const char* db::value(db_entry* entry) {
        return entry->value;
    }

    void db::set(database* db, const std::string& key, const std::string& value) {
        set(db, key, value.c_str());
    }

    void db::set(database* db, const std::string& key, const char* value) {
        auto* entry_ref = get_entry(db, key);

        if (entry_ref == nullptr) {
            return;
        }

        update(entry_ref, value);
    }

    const char* db::get(database* db, const std::string& key) {
        auto* entry_ref = find_entry(db, key);

        if (entry_ref == nullptr) {
            return nullptr;
        }

        return entry_ref->value;
    }

    void db::remove(database* db, const std::string& key) {
        for (int i = 0; i < DB_MAX_ENTRIES; i++) {
            auto* entry = &db->entries[i];

            if (entry->is_active == false || entry->name != key) {
                continue;
            }

            entry->is_active = false;
            break;
        }
    }
}
