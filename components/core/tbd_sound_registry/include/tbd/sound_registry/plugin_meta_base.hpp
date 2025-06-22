#pragma once

#include <tbd/sound_processor.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>
#include <tbd/parameter_types.hpp>

#include <atomic>

namespace tbd::sound_registry {

struct PluginMetaBase : sound_processor::SoundProcessor {
    explicit PluginMetaBase(const parameters::PluginID id) : id_(id), operation_cleared_(true) {}
    virtual ~PluginMetaBase() = default;

    virtual parameters::PluginID id() const { return id_; };

    Error map_param(const parameters::ParameterID parameter_id, const parameters::InputID input_id) {
        if (!parameters::verify_mapping(id_, parameter_id, input_id)) {
            return false;
        }

        if (!operation_cleared_) {
            return false;
        }
        operation_.type = WriteOperation::SET_MAPPING;
        operation_.parameter = parameter_id;
        operation_.value.int_param = input_id;
        operation_cleared_ = false;
        return true;
    };

    Error set_param(const parameters::ParameterID parameter_id, const int_par value) {
        if (!parameters::verify_int_parameter(id_, parameter_id)) {
            return false;
        }

        if (!operation_cleared_) {
            return false;
        }
        operation_.type = WriteOperation::SET_INT;
        operation_.parameter = parameter_id;
        operation_.value.int_param = value;
        operation_cleared_ = false;
        return true;
    };

    Error set_param(const parameters::ParameterID parameter_id, const uint_par value) {
        if (!parameters::verify_uint_parameter(id_, parameter_id)) {
            return false;
        }

        if (!operation_cleared_) {
            return false;
        }
        operation_.type = WriteOperation::SET_UINT;
        operation_.parameter = parameter_id;
        operation_.value.uint_param = value;
        operation_cleared_ = false;
        return true;
    };



    Error set_param(const parameters::ParameterID parameter_id, const float_par value) {
        if (!parameters::verify_float_parameter(id_, parameter_id)) {
            return false;
        }

        if (!operation_cleared_) {
            return false;   
        }
        operation_.type = WriteOperation::SET_FLOAT;
        operation_.parameter = parameter_id;
        operation_.value.float_param = value;
        operation_cleared_ = false;
        return true;
    };

    Error set_param(const parameters::ParameterID parameter_id, const trigger_par value) {
        if (!parameters::verify_trigger_parameter(id_, parameter_id)) {
            return false;
        }

        if (!operation_cleared_) {
            return false;
        }
        operation_.type = WriteOperation::SET_TRIGGER;
        operation_.parameter = parameter_id;
        operation_.value.trigger_param = value;
        operation_cleared_ = false;
        return true;
    };

protected:
    struct WriteOperation{
        enum {
            SET_MAPPING,
            SET_INT,
            SET_UINT,
            SET_TRIGGER,
            SET_FLOAT,
        } type;
        parameters::ParameterID parameter;
        union {
            parameters::Mapping mapping;
            int32_t int_param;
            uint32_t uint_param;
            bool trigger_param;
            float float_param;
        } value;
    } operation_;

    parameters::PluginID id_;
    std::atomic<bool> operation_cleared_;
};

}
