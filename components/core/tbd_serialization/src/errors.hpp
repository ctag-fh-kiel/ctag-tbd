#pragma once

#include <tbd/errors.hpp>

TBD_NEW_ERR(SERIALIZATION_BAD_TAG, "failed to decode message tag")
TBD_NEW_ERR(SERIALIZATION_FAILED_TAG_WRITE, "failed encode message tag")
TBD_NEW_ERR(SERIALIZATION_ZERO_TAG, "zero tag in encoded message")
TBD_NEW_ERR(SERIALIZATION_UNKNOWN_FIELD, "tag number does not match any defined field")

TBD_NEW_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_INT, "wire type is not suitable for int");
TBD_NEW_ERR(SERIALIZATION_ENCODING_INT_FAILED, "failed to encode int value");
TBD_NEW_ERR(SERIALIZATION_DECODING_INT_FAILED, "failed to decode int value");


TBD_NEW_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_UINT, "wire type is not suitable for uint");
TBD_NEW_ERR(SERIALIZATION_ENCODING_UINT_FAILED, "failed to encode uint value");
TBD_NEW_ERR(SERIALIZATION_DECODING_UINT_FAILED, "failed to decode uint value");


TBD_NEW_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_FLOAT, "wire type is not suitable for float")
TBD_NEW_ERR(SERIALIZATION_ENCODING_FLOAT_FAILED, "failed to encode float value")
TBD_NEW_ERR(SERIALIZATION_DECODING_FLOAT_FAILED, "failed to decode float value")
TBD_NEW_ERR(SERIALIZATION_UFLOAT_VALUE_FOR_ENCODING_IS_NEGATIVE, "can not encode negative ufloat value")
TBD_NEW_ERR(SERIALIZATION_DECODED_UFLOAT_VALUE_IS_NEGATIVE, "decoded ufloat value is negative")

TBD_NEW_ERR(SERIALIZATION_BAD_WIRE_TYPE_FOR_BOOL, "wire type is not suitable for bool");
TBD_NEW_ERR(SERIALIZATION_ENCODING_BOOL_FAILED, "failed to encode bool value");
TBD_NEW_ERR(SERIALIZATION_DECODING_BOOL_FAILED, "failed to decode bool value");

TBD_NEW_ERR(SERIALIZATION_FAILED_STRING_READ, "failed to read string from buffer")
TBD_NEW_ERR(SERIALIZATION_FAILED_STRING_WRITE, "failed to write string to buffer")

TBD_NEW_ERR(SERIALIZATION_DETERMINE_SUBMESSAGE_SIZE_FAILED, "can not determine size of submessage")
TBD_NEW_ERR(SERIALIZATION_WRITE_SUBMESSAGE_SIZE_FAILED, "can not determine size of submessage")
TBD_NEW_ERR(SERIALIZATION_BAD_SUBMESSAGE_WIRE_TYPE, "submessage wire type is not LEN")
TBD_NEW_ERR(SERIALIZATION_BAD_SUBMESSAGE_LENGTH, "failed to decode submessage length")

