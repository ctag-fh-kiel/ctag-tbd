#pragma once

#include <tbd/errors.hpp>

TBD_NEW_ERR(SERIALIZATION_BAD_TAG, "failed to decode message tag")
TBD_NEW_ERR(SERIALIZATION_FAILED_TAG_WRITE, "failed encode message tag")
TBD_NEW_ERR(SERIALIZATION_ZERO_TAG, "zero tag in encoded message")
TBD_NEW_ERR(SERIALIZATION_DETERMINE_SUBMESSAGE_SIZE_FAILED, "can not determine size of submessage")
TBD_NEW_ERR(SERIALIZATION_WRITE_SUBMESSAGE_SIZE_FAILED, "can not determine size of submessage")
TBD_NEW_ERR(SERIALIZATION_BAD_SUBMESSAGE_WIRE_TYPE, "submessage wire type is not LEN")
TBD_NEW_ERR(SERIALIZATION_BAD_SUBMESSAGE_LENGTH, "failed to decode submessage length")
TBD_NEW_ERR(SERIALIZATION_FAILED_STRING_READ, "failed to read string from buffer")
TBD_NEW_ERR(SERIALIZATION_FAILED_STRING_WRITE, "failed to write string to buffer")
