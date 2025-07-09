#pragma once

#include "pb_decode.h"

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include <pb_encode.h>

namespace tbd::serialization {

template<par_tags::ParamTypeTag type>
Error write_par(uint8_t field_number, par_tags::param_type_from_tag<type> value, pb_ostream_t& stream);

template<class MessageT>
Error decode_message(MessageT& dto, pb_istream_s& stream);

template<>
inline Error write_par<par_tags::INT_PARAM>(const uint8_t field_number, const int_par value, pb_ostream_t& stream) {
    pb_encode_tag(&stream, PB_WT_VARINT, field_number);
    pb_encode_svarint(&stream, value);
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::UINT_PARAM>(const uint8_t field_number, const uint_par value, pb_ostream_t& stream) {
    pb_encode_tag(&stream, PB_WT_VARINT, field_number);
    pb_encode_varint(&stream, value);
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::FLOAT_PARAM>(const uint8_t field_number, const float_par value, pb_ostream_t& stream) {
    pb_encode_tag(&stream, PB_WT_VARINT, field_number);
    pb_encode_fixed32(&stream, &value);
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::UFLOAT_PARAM>(const uint8_t field_number, const ufloat_par value, pb_ostream_t& stream) {
    if (value < 0) {
        return errors::FAILURE;
    }

    pb_encode_tag(&stream, PB_WT_VARINT, field_number);
    pb_encode_fixed32(&stream, &value);
    return TBD_OK;
}

template<>
inline Error write_par<par_tags::TRIGGER_PARAM>(const uint8_t field_number, const trigger_par value, pb_ostream_t& stream) {
    pb_encode_tag(&stream, PB_WT_VARINT, field_number);
    pb_encode_varint(&stream, value);
    return TBD_OK;
}

}