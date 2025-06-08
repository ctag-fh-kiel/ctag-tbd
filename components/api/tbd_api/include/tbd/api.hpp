#pragma once

#include <tbd/api/api_adapters.hpp>
#include <tbd/errors.hpp>

TBD_NEW_ERR(API_WRONG_PACKET_TYPE, "packet type does not match handler");
TBD_NEW_ERR(API_BAD_ENDPOINT, "invalid endpoint ID");
TBD_NEW_ERR(API_RESPONSE_BUFFER_SIZE, "output buffer too small for response payload");
TBD_NEW_ERR(API_DECODE, "failed to deserialize message");
TBD_NEW_ERR(API_ENCODE, "failed to serialize message");
TBD_NEW_ERR(API_BAD_ERROR, "invalid error ID");

namespace tbd::api {

template<class tag, bool allow_multithreading>
using ResponseWriter = impl::ResponseWriter<tag, allow_multithreading>;

template<class tag, bool allow_multithreading>
using EventWriter = impl::EventWriter<tag, allow_multithreading>;

template<class tag, bool allow_multithreading>
using ApiPacketHandler = impl::ApiPacketHandler<tag, allow_multithreading>;

template<PacketInputStream InputStreamT, PacketOutputStream OutputStreamT>
using ApiStreamHandler = impl::ApiStreamHandler<InputStreamT, OutputStreamT>;

}
