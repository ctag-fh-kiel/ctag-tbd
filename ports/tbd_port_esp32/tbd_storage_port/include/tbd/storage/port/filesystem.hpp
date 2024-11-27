/**
 * @brief drop-in replacement for subset of std::filesystem
 */
#pragma once

#include <string>
#include <optional>
#include <dirent.h>
#include <sys/stat.h>
#include <tbd/logging.hpp>


namespace tbd::storage::filesystem {

struct path final {
    friend struct directory_iterator;
    friend struct directory_entry;
    friend bool is_directory(const path& path);
    friend bool is_regular_file(const path& path);
    friend bool exists(const path& path);

    path(const char* path_str) : _path(path_str) {}

    path(const std::string& path_str) : _path(path_str) {}
    path(std::string&& path_str) : _path(std::move(path_str)) {}

    path(const path& other) : _path(other._path) {}
    path(path&& other) : _path(other._path) {}

    path& operator=(const char* path_str) {
        _path = path_str;
        return *this;
    }

    path& operator=(const path& other) {
        _path = other._path;
        return *this;
    }

    path& operator=(path&& other) {
        _path = std::move(other._path);
        return *this;
    }

    path& operator/= (const path& other) {
        _path += '/' + other._path; 
        return *this;   
    }

    path operator/ (const path& other) {
        path new_path(*this);
        return new_path /= other;   
    }

    path filename() const {
        size_t lastSlash = _path.rfind("/");
        if (lastSlash != std::string::npos){
            return _path.substr(lastSlash + 1);
            // std::string folder = _path.substr(0, lastSlash);
        }
        return _path;
    }

    operator const char* () {
        return c_str();
    }

    operator std::string () {
        return string();
    }

    const std::string& string() const {
        return _path;
    }

    const char* c_str() const {
        return _path.c_str();
    }

private:
    bool is_null() {
        return _path.empty();
    }

    void set_null() {
        _path.clear();
    }

    path() {
        // null path
    }
 
    std::string _path;
};

struct directory_entry {
    friend struct directory_iterator;

    tbd::storage::filesystem::path& path() { return _path; }
    const tbd::storage::filesystem::path& path() const { return _path; }
private:
    tbd::storage::filesystem::path _path;
};

inline bool is_directory(const path& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        TBD_LOGE("storages", "failed to stat file %s", path.c_str());
        return false;
    }
    return S_ISDIR(info.st_mode);
}

inline bool is_regular_file(const path& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        TBD_LOGE("storages", "failed to stat file %s", path.c_str());
        return false;
    }
    return S_ISREG(info.st_mode);
}

inline bool exists(const path& path) {
    struct stat info;
    return stat(path.c_str(), &info) != 0;
}

struct directory_iterator {
    directory_iterator(const path& dir) {
        open_dir(dir._path);
    }

    directory_iterator(const std::string& dir) {
        open_dir(dir);
    }

    ~directory_iterator() {
        
    }

    directory_iterator& begin() {
        if (!_current._path.is_null()) {
            TBD_LOGE("storage", "multiple calls to directory operator begin");
        }
        return *this;
    }

    const directory_iterator& end() {
        return _end_token;
    }

    directory_iterator& operator++() {
        if (_handle == nullptr) {
            return *this;
        }

        dirent* entry;
        if ((entry = readdir(_handle)) == nullptr) {
            _current._path.set_null();
            close_dir();
        }

        _current._path = entry->d_name;
        return *this;
    }

    directory_entry& operator*() {
        return _current;
    }

    const directory_entry& operator*() const {
        return _current;
    }

    bool operator==(const directory_iterator& other) const {
        if (this->_handle == nullptr && other._handle == nullptr) {
            return true;
        }
        return true;
    }

    bool operator!=(const directory_iterator& other) const {
        if (this->_handle == nullptr && other._handle == nullptr) {
            return false;
        }
        return true;
    }

    private:
        directory_iterator() : _handle(nullptr) {
            // end iterator
        }
        static const directory_iterator _end_token;

        void open_dir(const std::string& dir) {
            _handle = opendir(dir.c_str());
            if (_handle == nullptr) {
                TBD_LOGE("storage", "invalid directory: %s", dir.c_str());
            }
        };

        void close_dir() {
            if (_handle != nullptr) {
                closedir(_handle);
                _handle = nullptr;
            }
        }

        DIR* _handle;
        directory_entry _current;
};

}