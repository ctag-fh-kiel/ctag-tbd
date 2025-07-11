#pragma once

#include "pb_decode.h"

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include <pb_encode.h>

namespace tbd::serialization {

template<par_tags::ParamTypeTag type>
Error write_par(uint8_t field_number, const par_tags::param_type_from_tag<type>& value, pb_ostream_t& stream);



template<class FieldT>
Error encode_field(uint8_t field_number, const FieldT& dto, pb_ostream_t& stream);

template<class MessageT>
Error encode_message(const MessageT& dto, pb_ostream_t& stream);

template<class MessageT>
Error decode_message(MessageT& dto, pb_istream_t& stream);

template<par_tags::ParamTypeTag type>
Error decode_par_data(pb_wire_type_t wire_type, par_tags::param_type_from_tag<type>& value, pb_istream_t& stream);

template<class MessageT>
Error decode_submessage_data(pb_wire_type_t wire_type, MessageT& dto, pb_istream_t& stream) {
    if (wire_type != PB_WT_STRING) { return TBD_ERR(SERIALIZATION_BAD_SUBMESSAGE_WIRE_TYPE); }
    uint32_t size;
    if (!pb_decode_varint32(&stream, &size)) { return TBD_ERR(SERIALIZATION_BAD_SUBMESSAGE_LENGTH); }
    return decode_message(dto, stream);
}

template<>
inline Error write_par<par_tags::INT_PARAM>(const uint8_t field_number, const int_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_svarint(&stream, value)) { return errors::FAILURE; }
    return TBD_OK;
}

template<>
inline Error decode_par_data<par_tags::INT_PARAM>(const pb_wire_type_t wire_type, int_par& value, pb_istream_t& stream) {
    if (wire_type == PB_WT_VARINT) {

    } else if (wire_type == PB_WT_32BIT) {

    } else {
        return errors::FAILURE;
    }
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::UINT_PARAM>(const uint8_t field_number, const uint_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_varint(&stream, value)) { return errors::FAILURE; }
    return TBD_OK;
}

template<>
inline Error decode_par_data<par_tags::UINT_PARAM>(pb_wire_type_t wire_type, uint_par& value, pb_istream_t& stream) {
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::FLOAT_PARAM>(const uint8_t field_number, const float_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_fixed32(&stream, &value)) { return errors::FAILURE; }
    return TBD_OK;
}

template<>
inline Error decode_par_data<par_tags::FLOAT_PARAM>(pb_wire_type_t wire_type, float& value, pb_istream_t& stream) {
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::UFLOAT_PARAM>(const uint8_t field_number, const ufloat_par& value, pb_ostream_t& stream) {
    if (value < 0) {
        return errors::FAILURE;
    }

    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_fixed32(&stream, &value)) { return errors::FAILURE; }
    return TBD_OK;
}

template<>
inline Error decode_par_data<par_tags::UFLOAT_PARAM>(pb_wire_type_t wire_type, ufloat_par& value, pb_istream_t& stream) {
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::TRIGGER_PARAM>(const uint8_t field_number, const trigger_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_varint(&stream, value)) { return errors::FAILURE; }
    return TBD_OK;
}

template<>
inline Error decode_par_data<par_tags::TRIGGER_PARAM>(pb_wire_type_t wire_type, bool& value, pb_istream_t& stream) {
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::STR_PARAM>(const uint8_t field_number, const str_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_STRING, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_string(&stream, reinterpret_cast<const pb_byte_t*>(value.data()), value.length())) {
        return TBD_ERR(SERIALIZATION_FAILED_STRING_WRITE);
    }
    return TBD_OK;
}

template<>
inline Error decode_par_data<par_tags::STR_PARAM>(pb_wire_type_t wire_type, str_par& value, pb_istream_t& stream) {
    if (wire_type != PB_WT_STRING) {
        return TBD_ERR(SERIALIZATION_BAD_SUBMESSAGE_WIRE_TYPE);
    }
    uint32_t size;
    if (!pb_decode_varint32(&stream, &size)) {
        return TBD_ERR(SERIALIZATION_BAD_SUBMESSAGE_LENGTH);
    }
    if (size == 0) { return TBD_OK; }
    value.resize(size);
    if (!pb_read(&stream, reinterpret_cast<pb_byte_t*>(value.data()), size)) {
        return TBD_ERR(SERIALIZATION_FAILED_STRING_READ);
    }
    return TBD_OK;
}

}