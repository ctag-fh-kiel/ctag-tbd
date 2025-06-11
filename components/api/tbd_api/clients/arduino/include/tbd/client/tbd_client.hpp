#pragma once

#include <tbd/client/client.hpp>
#include <tbd/client/tbd_rpc.hpp>
#include <tbd/client/tbd_event.hpp>

using TBDClient = tbd::client::Client<tbd::client::RPC, tbd::client::Event>;
