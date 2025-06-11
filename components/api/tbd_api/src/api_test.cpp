#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>
#include <tbd/logging.hpp>

#include <api_types.pb.h>


namespace tbd {

[[tbd::endpoint]]
Error foo_rpc(const Foo& foo) {
    TBD_LOGI("foo_rpc", "foo1 %i", foo.foo1);
    TBD_LOGI("foo_rpc", "foo2 %i", foo.foo2);
    return TBD_OK;
}

[[tbd::event]]
void some_trigger_event();

[[tbd::responder(event="some_trigger_event")]]
void handle_some_event1() {
    TBD_LOGI("some event", "called handler 1");
}

[[tbd::responder(event="some_trigger_event")]]
void handle_some_event2() {
    TBD_LOGI("some event", "called handler 2");
}

[[tbd::responder(event="some_trigger_event")]]
void handle_some_event3() {
    TBD_LOGI("some event", "called handler 3");
}

[[tbd::endpoint]]
Error test_some_trigger_event() {
    some_trigger_event();
    return TBD_OK;
}

[[tbd::event]]
void blub_event(const uint_par& a, const float_par& b);

[[tbd::endpoint]]
Error test_blub_event() {
    blub_event(17, 232.23);
    return TBD_OK;
}

[[tbd::event]]
void foo_event(const uint_par& a, const float_par& b);

[[tbd::endpoint]]
Error test_foo_event() {
    foo_event(9, 3.2);
    return TBD_OK;
}

[[tbd::responder(event="foo_event")]]
void foo_responder1(const uint_par& a, const float_par& b) {
    TBD_LOGI("foo", "responder1: %i, %f", a, b);
}

[[tbd::responder(event="foo_event")]]
Error foo_responder2(const uint_par& a, const float_par& b) {
    TBD_LOGI("foo", "responder2: %i, %f", a, b);
    return TBD_OK;
}

[[tbd::responder(event="foo_event")]]
void foo_responder3(const uint_par& a, const float_par& b) {
    TBD_LOGI("foo", "responder3: %i, %f", a, b);
}

[[tbd::event]]
void bar_event(const Bar& event);

[[tbd::endpoint]]
Error test_bar_event() {
    Bar bar;
    bar.bar1 = 90.2;
    bar.bar2 = -23;
    bar_event(bar);
    return TBD_OK;
}

[[tbd::responder(event="bar_event")]]
Error bar_responder1(const Bar& event) {
    return TBD_OK;
}

[[tbd::responder(event="bar_event")]]
void bar_responder2(const Bar& event) {

}

}