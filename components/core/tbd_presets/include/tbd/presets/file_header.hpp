#pragma once

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include "serializables.hpp"

namespace tbd::presets {

constexpr uint32_t HEADER_START_BYTES = 0xc12304de;
constexpr uint32_t HEADER_END_BYTES = 0xab8b7601;
constexpr uint32_t FILE_END_BYTES = 0xeaf7ddb7;

struct FileHeader {
    TypeID type_id;
    uint_par data_size;
};

inline Error read_file_header(std::istream& in_stream, FileHeader& header) {
    uint32_t start_bytes;
    if (in_stream >> start_bytes; in_stream.fail() || start_bytes != HEADER_START_BYTES) {
        return TBD_ERRMSG(PRESETS_BAD_START_BYTES, "bad start bytes in file header");
    }
    if (in_stream >> header.type_id; in_stream.fail()) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_READ_TYPE_ID, "could not read type ID from file header");
    }
    if (in_stream >> header.data_size; in_stream.fail()) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_READ_DATA_SIZE, "could not read data size from file header");
    }
    uint_par header_end_bytes;
    if (in_stream >> header_end_bytes; in_stream.fail() || header_end_bytes != HEADER_END_BYTES) {
        return TBD_ERRMSG(PRESETS_BAD_HEADER_END_BYTES, "bad header end bytes in file header");
    }
    return TBD_OK;
}

template<typename SerializableT>
Error read_from_stream(std::istream& in_stream, SerializableT& obj) {
    FileHeader header;
    if (const auto err = read_file_header(in_stream, header); err != TBD_OK) {
        return err;
    }
    if (header.type_id != Serializable<SerializableT>::type_id()) {
        return TBD_ERRMSG(PRESETS_ATTEMPTING_TO_READ_WRONG_TYPE_FROM_FILE, "type ID in file and object mismatch");
    }
    if (const auto err = read_serializable(in_stream, obj); err != TBD_OK) {
        return err;
    }
    uint32_t file_end_bytes;
    if (in_stream >> file_end_bytes; in_stream.fail() || file_end_bytes != FILE_END_BYTES) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_READ_FILE_END_BYTES, "could not read file end bytes from file");
    }
    return TBD_OK;
}

inline Error write_file_header(std::ostream& out_stream, const FileHeader& header) {
    if (out_stream << HEADER_START_BYTES; out_stream.fail()) {
        return TBD_ERRMSG(PRESETS_BAD_START_BYTES, "failed to write start bytes to file header");
    }
    if (out_stream << header.type_id; out_stream.fail()) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_WRITE_TYPE_ID, "failed to write type ID to file header");
    }
    if (out_stream << header.data_size; out_stream.fail()) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_WRITE_DATA_SIZE, "failed to write data size to file header");
    }
    if (out_stream << HEADER_END_BYTES; out_stream.fail()) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_WRITE_HEADER_END_BYTES, "failed to write header end bytes to file header");
    }
    return TBD_OK;
}

template<typename SerializableT>
Error write_to_stream(std::ostream& out_stream, const SerializableT& obj) {
    const auto data_size = Serializable<SerializableT>::serialized_size(obj);
    const FileHeader header { Serializable<SerializableT>::type_id(), data_size};
    if (const auto err = write_file_header(out_stream, header); err != TBD_OK) {
        return err;
    }
    if (const auto err = write_serializable(out_stream, obj); err != TBD_OK) {
        return err;
    }
    if (out_stream << FILE_END_BYTES; out_stream.fail()) {
        return TBD_ERRMSG(PRESETS_FAILED_TO_WRITE_FILE_END_BYTES, "failed to write file end bytes to file");
    }
    return TBD_OK;
}

}
