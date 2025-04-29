#include <tbd/sound_processor.hpp>
#include <tbd/sound_processor/parameters.hpp>

#include <atomic>

namespace tbd::audio {

class DynamicSoundProcessor {
public:
    DynamicSoundProcessor(parameters::PluginID plugin_id) : id_(plugin_id), operation_cleared_(true) {}
    virtual ~DynamicSoundProcessor() {} 

    virtual parameters::PluginID id() const {
        return id_;
    };

    bool map_param(parameters::ParameterID parameter_id, parameters::InputID input_id) {
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
    };

    bool set_param(parameters::ParameterID parameter_id, int32_t value) {
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
    };

    bool set_param(parameters::ParameterID parameter_id, uint32_t value) {
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
    };

    bool set_param(parameters::ParameterID parameter_id, bool value) {
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
    };

    bool set_param(parameters::ParameterID parameter_id, float value) {
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
    };

protected:
    parameters::PluginID id_;

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

    std::atomic<bool> operation_cleared_;
};

}
