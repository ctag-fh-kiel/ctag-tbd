#pragma once

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include <pb_encode.h>


namespace tbd::serialization {

// encode classes //

template<class MessageT>
Error encode_message(const MessageT& dto, pb_ostream_t& stream);

template<class FieldT>
Error encode_class_field(uint8_t field_number, const FieldT& dto, pb_ostream_t& stream);

// encode parameters //

template<par_tags::ParamTypeTag type>
Error encode_par_field(uint8_t field_number, const par_tags::param_type_from_tag<type>& value, pb_ostream_t& stream);

template<>
inline Error encode_par_field<par_tags::INT_PARAM>(const uint8_t field_number, const int_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_svarint(&stream, value)) {
        return TBD_ERR(SERIALIZATION_ENCODING_INT_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error encode_par_field<par_tags::UINT_PARAM>(const uint8_t field_number, const uint_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_varint(&stream, value)) {
        return TBD_ERR(SERIALIZATION_ENCODING_UINT_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error encode_par_field<par_tags::FLOAT_PARAM>(const uint8_t field_number, const float_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_fixed32(&stream, &value)) {
        return TBD_ERR(SERIALIZATION_ENCODING_FLOAT_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error encode_par_field<par_tags::UFLOAT_PARAM>(const uint8_t field_number, const ufloat_par& value, pb_ostream_t& stream) {
    if (value < 0) {
        return TBD_ERR(SERIALIZATION_UFLOAT_VALUE_FOR_ENCODING_IS_NEGATIVE);
    }
    return encode_par_field<par_tags::UFLOAT_PARAM>(field_number, value, stream);
}

template<>
inline Error encode_par_field<par_tags::TRIGGER_PARAM>(const uint8_t field_number, const trigger_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_VARINT, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_varint(&stream, value)) {
        return TBD_ERR(SERIALIZATION_ENCODING_BOOL_FAILED);
    }
    return TBD_OK;
}

template<>
inline Error encode_par_field<par_tags::STR_PARAM>(const uint8_t field_number, const str_par& value, pb_ostream_t& stream) {
    if (!pb_encode_tag(&stream, PB_WT_STRING, field_number)) {
        return TBD_ERR(SERIALIZATION_FAILED_TAG_WRITE);
    }
    if (!pb_encode_string(&stream, reinterpret_cast<const pb_byte_t*>(value.data()), value.length())) {
        return TBD_ERR(SERIALIZATION_FAILED_STRING_WRITE);
    }
    return TBD_OK;
}

}