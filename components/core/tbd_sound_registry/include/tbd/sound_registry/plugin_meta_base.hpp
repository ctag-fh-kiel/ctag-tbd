#pragma once

#include <tbd/sound_processor.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>
#include <tbd/parameter_types.hpp>

#include <atomic>
#include <functional>

namespace tbd::sound_registry {

struct PluginMetaBase : sound_processor::SoundProcessor {
    explicit PluginMetaBase(const parameters::PluginID id) : id_(id), operation_pending_(false) {}
    ~PluginMetaBase() override = default;

    virtual parameters::PluginID id() const { return id_; };

    // value setters //

    Error set_param(const parameters::ParameterID parameter_id, const int_par value) {
        if (const auto err = parameters::verify_int_parameter(id_, parameter_id); err != TBD_OK) {
            return err;
        }
        if (operation_pending_.test_and_set()) {
            return TBD_ERR(SOUND_REGISTRY_PENDING_OPERATION);
        }

        operation_.type = WriteOperation::SET_VALUE;
        operation_.parameter = parameter_id;
        operation_.payload.param = {.int_value = value};
        return TBD_OK;
    }

    Error set_param(const parameters::ParameterID parameter_id, const uint_par value) {
        if (const auto err = parameters::verify_uint_parameter(id_, parameter_id); err != TBD_OK) {
            return err;
        }
        if (operation_pending_.test_and_set()) {
            return TBD_ERR(SOUND_REGISTRY_PENDING_OPERATION);
        }

        operation_.type = WriteOperation::SET_VALUE;
        operation_.parameter = parameter_id;
        operation_.payload.param = {.uint_value = value};
        return TBD_OK;
    }

    Error set_param(const parameters::ParameterID parameter_id, const float_par value) {
        if (const auto err = parameters::verify_float_parameter(id_, parameter_id); err != TBD_OK) {
            return err;
        }
        if (operation_pending_.test_and_set()) {
            return TBD_ERR(SOUND_REGISTRY_PENDING_OPERATION);
        }

        operation_.type = WriteOperation::SET_VALUE;
        operation_.parameter = parameter_id;
        operation_.payload.param = {.float_value = value};
        return TBD_OK;
    }

    Error set_param(const parameters::ParameterID parameter_id, const trigger_par value) {
        if (const auto err = parameters::verify_trigger_parameter(id_, parameter_id); err != TBD_OK) {
            return err;
        }
        if (operation_pending_.test_and_set()) {
            return TBD_ERR(SOUND_REGISTRY_PENDING_OPERATION);
        }

        operation_.type = WriteOperation::SET_VALUE;
        operation_.parameter = parameter_id;
        operation_.payload.param = {.trigger_value = value};
        return TBD_OK;
    }

    // parameter mapping //

    Error map_param(const parameters::ParameterID parameter_id, const parameters::InputID input_id) {
        if (const auto err = parameters::verify_mapping(id_, parameter_id, input_id);
            err != TBD_OK)
        {
            return err;
        }
        if (operation_pending_.test_and_set()) {
            return TBD_ERR(SOUND_REGISTRY_PENDING_OPERATION);
        }

        operation_.type = WriteOperation::SET_MAPPING;
        operation_.parameter = parameter_id;
        operation_.payload.mapping = input_id;
        return TBD_OK;
    }

protected:
    struct WriteOperation {
        enum {
            NOOP        = 0,
            SET_VALUE   = 1,
            SET_MAPPING = 2,
            CALL        = 3,
        } type;
        parameters::ParameterID parameter;
        union {
            parameters::Mapping mapping;
            ParamValue param;
        } payload;
    } operation_;

    parameters::PluginID id_;
    std::atomic_flag operation_pending_;
};

}
