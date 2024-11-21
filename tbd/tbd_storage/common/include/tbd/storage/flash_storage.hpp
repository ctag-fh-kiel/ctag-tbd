#pragma once

#include <cstddef>
#include <cinttypes>


namespace tbd::storage {

using address_type = size_t;
constexpr auto address_size = sizeof(address_type);


/**
 *  low level stateless flash ROM storage interface
 */
struct FlashStorage {
    /**
     *  read raw data from any memory address in flash
     */
    void read(void* buffer, address_type address, address_type length);
};


/** 
 *   file like flash storage abstraction
 * 
 *   Just as with files, consecutive read and write operations are performed relative 
 *   starting at the end position of the previous read.
 * 
 *   @arg storage         flash device (physical device)
 *   @arg block_address   flash offset to start reading and writing (like a file or folder)
 *   @arg magic_number    hash or magic number to verify the starting offset points to
 *                        the data to be read (default 0 means no checking)
 */
template<
    FlashStorage& storage, 
    address_type block_address, 
    address_type magic_number = 0
>
struct FlashReader {
    FlashReader() {
        check_magic_number();
    }

    /**
     *  read data chunk into buffer, starting at current reader position
     */
    void read_chunk(void* buffer, address_type length) {
        storage.read(buffer, block_address + _offset, length);
        _offset += length;
    }

    /**
     *  read data as specific type, starting at current reader position
     */
    template<typename T>
    T read() {
        T res;
        auto type_size = sizeof(T);
        storage.read(&res, block_address + _offset, type_size);
        _offset += type_size;
        return res;
    }

    /**
     *   jump to any position in flash block
     * 
     *   @note: position is relative to block address
     */
    void seek(address_type position) {
        _offset = position;
    }

    /**
     *   get current position as offset of block address
     */
    address_type pos() const {
        return _offset;
    }

private:
    void check_magic_number() {
        if (magic_number != 0) {
            address_type head;
            storage.read(&head, block_address, address_size);
            if (head != magic_number) {
                _has_error = true;
            }
            _offset += address_size;
        }
    }

    bool _has_error = false;
    uint32_t _offset = 0;
};

extern FlashStorage default_flash;

}
