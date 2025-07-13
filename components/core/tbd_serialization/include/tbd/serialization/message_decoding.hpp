#pragma once

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include <pb_decode.h>


namespace tbd::serialization {

// decode classes //

template<class MessageT>
Error decode_message(MessageT& dto, pb_istream_t& stream);

template<class MessageT>
Error decode_submessage_data(pb_wire_type_t wire_type, MessageT& dto, pb_istream_t& stream) {
    if (wire_type != PB_WT_STRING) { return TBD_ERR(SERIALIZATION_BAD_SUBMESSAGE_WIRE_TYPE); }
    uint32_t size;
    if (!pb_decode_varint32(&stream, &size)) { return TBD_ERR(SERIALIZATION_BAD_SUBMESSAGE_LENGTH); }
    return decode_message(dto, stream);
}

// decode parameters //

template<par_tags::ParamTypeTag type>
Error decode_par(pb_wire_type_t wire_type, par_tags::param_type_from_tag<type>& value, pb_istream_t& stream);

template<>
inline Error decode_par<par_tags::INT_PARAM>(const pb_wire_type_t wire_type, int_par& value, pb_istream_t& stream) {
    if (wire_type != PB_WT_VARINT && wire_type != PB_WT_PACKED) {
        return TBD_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_INT);
    }
    if (!pb_decode_svarint(&stream, &value)) {
        return TBD_ERR(SERIALIZATION_DECODING_INT_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error decode_par<par_tags::UINT_PARAM>(pb_wire_type_t wire_type, uint_par& value, pb_istream_t& stream) {
    if (wire_type != PB_WT_VARINT && wire_type != PB_WT_PACKED) {
        return TBD_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_UINT);
    }
    if (!pb_decode_varint(&stream, &value)) {
        return TBD_ERR(SERIALIZATION_DECODING_UINT_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error decode_par<par_tags::FLOAT_PARAM>(pb_wire_type_t wire_type, float& value, pb_istream_t& stream) {
    if (wire_type != PB_WT_32BIT && wire_type != PB_WT_PACKED) {
        return TBD_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_FLOAT);
    }
    if (!pb_decode_fixed32(&stream, &value)) {
        return TBD_ERR(SERIALIZATION_DECODING_FLOAT_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error decode_par<par_tags::UFLOAT_PARAM>(pb_wire_type_t wire_type, ufloat_par& value, pb_istream_t& stream) {
    if (const auto err = decode_par<par_tags::FLOAT_PARAM>(wire_type, value, stream); err != TBD_OK) {
        return err;
    }
    if (value < 0) {
        return TBD_ERR(SERIALIZATION_DECODED_UFLOAT_VALUE_IS_NEGATIVE);
    }
    return TBD_OK;
}

template<>
inline Error decode_par<par_tags::TRIGGER_PARAM>(pb_wire_type_t wire_type, bool& value, pb_istream_t& stream) {
    if (wire_type != PB_WT_VARINT && wire_type != PB_WT_PACKED) {
        return TBD_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_BOOL);
    }
    if (!pb_decode_bool(&stream, &value)) {
        return TBD_ERR(SERIALIZATION_DECODING_BOOL_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error decode_par<par_tags::STR_PARAM>(pb_wire_type_t wire_type, str_par& value, pb_istream_t& stream) {
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