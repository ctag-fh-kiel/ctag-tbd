#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include <api_types.pb.h>


namespace tbd {

[[tbd::event]]
Error foo_event(const Foo& event);

[[tbd::event]]
Error bar_event(const Bar& event);

[[tbd::responder(event="tbd.bar_event")]]
void bar_responder1(const Bar& event) {

}

[[tbd::responder(event="tbd.bar_event")]]
void bar_responder2(const Bar& event) {

}

}