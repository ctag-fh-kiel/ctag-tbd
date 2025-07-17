#include <tbd/api/dtos.hpp>

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>
#include <tbd/logging.hpp>


namespace tbd::api_examples {

// event with no args and no responders //

[[tbd::event]]
void no_arg_event();


// event with no args and responders //

[[tbd::event]]
void used_no_arg_event();

[[tbd::responder(event="used_no_arg_event")]]
void handle_some_event1() {
    TBD_LOGI("some event", "called handler 1");
}

[[tbd::responder(event="used_no_arg_event")]]
void handle_some_event2() {
    TBD_LOGI("some event", "called handler 2");
}

[[tbd::responder(event="used_no_arg_event")]]
void handle_some_event3() {
    TBD_LOGI("some event", "called handler 3");
}


// param arg event //

[[tbd::event]]
void param_arg_event(const float_par& arg);

[[tbd::event]]
void used_param_arg_event(const float_par& arg);

[[tbd::responder(event="used_param_arg_event")]]
Error param_arg_responder(const float_par& arg) {
    TBD_LOGI("param_arg_responder", "%f", arg);
    return TBD_OK;
}

// multi param arg event //

[[tbd::event]]
void multi_param_arg_event(const float_par& arg1, const trigger_par& arg2);

[[tbd::event]]
void used_multi_param_arg_event(const float_par& arg1, const trigger_par& arg2);

[[tbd::responder(event="used_multi_param_arg_event")]]
Error multi_param_arg_responder(const float_par& arg1, const trigger_par& arg2) {
    TBD_LOGI("param_arg_responder", "arg1 %f", arg1);
    TBD_LOGI("param_arg_responder", "arg2 %i", arg2);
    return TBD_OK;
}

// cls and param arg event //

[[tbd::event]]
void cls_and_param_arg_event(const float_par& arg1, const api::SimpleCls& arg2, const trigger_par& arg3);

[[tbd::event]]
void used_cls_and_param_arg_event(const float_par& arg1, const api::SimpleCls& arg2, const trigger_par& arg3);

[[tbd::responder(event="used_cls_and_param_arg_event")]]
Error cls_and_param_arg_responder(const float_par& arg1, const api::SimpleCls& arg2, const trigger_par& arg3) {
    TBD_LOGI("param_arg_responder", "arg1 %f", arg1);
    TBD_LOGI("param_arg_responder", "arg2.simple1 %i", arg2.simple1);
    TBD_LOGI("param_arg_responder", "arg2.simple2 %i", arg2.simple2);
    TBD_LOGI("param_arg_responder", "arg3 %i", arg3);
    return TBD_OK;
}

// single struct arg event //

[[tbd::event]]
void simple_cls_arg_event(const api::SimpleCls& arg);

[[tbd::event]]
void used_simple_cls_arg_event(const api::SimpleCls& arg);

[[tbd::responder(event="used_simple_cls_arg_event")]]
Error simple_cls_arg_responder(const api::SimpleCls& arg) {
    TBD_LOGI("simple_cls_arg_responder", "simple1: %i", arg.simple1);
    TBD_LOGI("simple_cls_arg_responder", "simple2: %i", arg.simple2);
    return TBD_OK;
}

// nested cls single arg event //

[[tbd::event]]
void complex_cls_arg_event(const api::ComplexCls& arg);

[[tbd::event]]
void used_complex_cls_arg_event(const api::ComplexCls& arg);

[[tbd::responder(event="used_complex_cls_arg_event")]]
Error complex_cls_arg_responder(const api::ComplexCls& arg) {
    TBD_LOGI("complex_cls_arg_responder", "complex1 %i", arg.complex1);
    TBD_LOGI("complex_cls_arg_responder", "complex2.simple1 %i", arg.complex2.simple1);
    TBD_LOGI("complex_cls_arg_responder", "complex2.simple2 %i", arg.complex2.simple2);
    TBD_LOGI("complex_cls_arg_responder", "complex2 %i", arg.complex3);
    return TBD_OK;
}

// complex nested cls single arg event //

[[tbd::event]]
void very_complex_arg_event(const api::VeryComplexCls& arg);

[[tbd::event]]
void used_very_complex_arg_event(const api::VeryComplexCls& arg);

[[tbd::responder(event="used_very_complex_arg_event")]]
Error very_complex_arg_responder(const api::VeryComplexCls& arg) {
    TBD_LOGI("very_complex_arg_responder", "vcomplex1 %f", arg.vcomplex1);
    TBD_LOGI("very_complex_arg_responder", "vcomplex2.complex1 %i", arg.vcomplex2.complex1);
    TBD_LOGI("very_complex_arg_responder", "vcomplex2.complex2.simple1 %i", arg.vcomplex2.complex2.simple1);
    TBD_LOGI("very_complex_arg_responder", "vcomplex2.complex2.simple2 %i", arg.vcomplex2.complex2.simple2);
    TBD_LOGI("very_complex_arg_responder", "vcomplex2.complex2 %i", arg.vcomplex2.complex3);
    TBD_LOGI("very_complex_arg_responder", "vcomplex3.simple1 %i", arg.vcomplex3.simple1);
    TBD_LOGI("very_complex_arg_responder", "vcomplex3.simple2 %i", arg.vcomplex3.simple2);
    TBD_LOGI("very_complex_arg_responder", "vcomplex4.complex1 %i", arg.vcomplex2.complex1);
    TBD_LOGI("very_complex_arg_responder", "vcomplex4.complex2.simple1 %i", arg.vcomplex2.complex2.simple1);
    TBD_LOGI("very_complex_arg_responder", "vcomplex4.complex2.simple2 %i", arg.vcomplex2.complex2.simple2);
    TBD_LOGI("very_complex_arg_responder", "vcomplex4.complex2 %i", arg.vcomplex2.complex3);
    TBD_LOGI("very_complex_arg_responder", "vcomplex6 %i", arg.vcomplex6);
    return TBD_OK;
}

}
