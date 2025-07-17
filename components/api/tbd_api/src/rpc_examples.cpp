#include <tbd/api/dtos.hpp>

#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>
#include <tbd/logging.hpp>


namespace tbd::api_examples {

// no arg RPC //

[[tbd::endpoint]]
Error no_arg_rpc() {
    TBD_LOGI("no arg rpc", "");
    return TBD_OK;
}

// param arg RPC //

[[tbd::endpoint]]
Error param_arg_rpc(const float_par& arg) {
    TBD_LOGI("param_arg_rpc", "%f", arg);
    return TBD_OK;
}

// multi param arg RPC //

[[tbd::endpoint]]
Error multi_param_arg_rpc(const float_par& arg1, const trigger_par& arg2) {
    TBD_LOGI("param_arg_rpc", "arg1 %f", arg1);
    TBD_LOGI("param_arg_rpc", "arg2 %i", arg2);
    return TBD_OK;
}

// cls and param arg RPC //

[[tbd::endpoint]]
Error cls_and_param_arg_rpc(const float_par& arg1, const api::SimpleCls& arg2, const trigger_par& arg3) {
    TBD_LOGI("param_arg_rpc", "arg1 %f", arg1);
    TBD_LOGI("param_arg_rpc", "arg2.simple1 %i", arg2.simple1);
    TBD_LOGI("param_arg_rpc", "arg2.simple2 %i", arg2.simple2);
    TBD_LOGI("param_arg_rpc", "arg3 %i", arg3);
    return TBD_OK;
}

// single struct arg RPC //

[[tbd::endpoint]]
Error simple_cls_arg_rpc(const api::SimpleCls& arg) {
    TBD_LOGI("simple_cls_arg_rpc", "simple1: %i", arg.simple1);
    TBD_LOGI("simple_cls_arg_rpc", "simple2: %i", arg.simple2);
    return TBD_OK;
}

// nested cls single arg RPC //

[[tbd::endpoint]]
Error complex_cls_arg_rpc(const api::ComplexCls& arg) {
    TBD_LOGI("complex_cls_arg_rpc", "complex1 %i", arg.complex1);
    TBD_LOGI("complex_cls_arg_rpc", "complex2.simple1 %i", arg.complex2.simple1);
    TBD_LOGI("complex_cls_arg_rpc", "complex2.simple2 %i", arg.complex2.simple2);
    TBD_LOGI("complex_cls_arg_rpc", "complex2 %i", arg.complex3);
    return TBD_OK;
}

// complex nested cls single arg RPC //

[[tbd::endpoint]]
Error very_complex_arg_rpc(const api::VeryComplexCls& arg) {
    TBD_LOGI("very_complex_arg_rpc", "vcomplex1 %f", arg.vcomplex1);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex2.complex1 %i", arg.vcomplex2.complex1);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex2.complex2.simple1 %i", arg.vcomplex2.complex2.simple1);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex2.complex2.simple2 %i", arg.vcomplex2.complex2.simple2);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex2.complex2 %i", arg.vcomplex2.complex3);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex3.simple1 %i", arg.vcomplex3.simple1);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex3.simple2 %i", arg.vcomplex3.simple2);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex4.complex1 %i", arg.vcomplex2.complex1);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex4.complex2.simple1 %i", arg.vcomplex2.complex2.simple1);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex4.complex2.simple2 %i", arg.vcomplex2.complex2.simple2);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex4.complex2 %i", arg.vcomplex2.complex3);
    TBD_LOGI("very_complex_arg_rpc", "vcomplex6 %i", arg.vcomplex6);
    return TBD_OK;
}

}