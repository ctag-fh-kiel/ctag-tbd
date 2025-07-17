#include <tbd/sound_registry/active_sound_processors.hpp>

#include <tbd/sound_processor/module.hpp>
#include <tbd/errors.hpp>


using tbd::sound_processor::tag;

namespace tbd::sound_registry {

namespace {

using namespace sound_processor::channels;
using namespace sound_registry::parameters;

constexpr size_t TOTAL_SIZE = TBD_SOUND_PROCESSOR_MAX_MEMORY;

/** Memory block to hold plugins and additional buffers in builtin RAM (SRAM).
 *
 *  Depending on whether a single stereo plugin outputs to both channels or there are two mono plugins for each channel
 *  there are two possible memory layouts:
 *
 *  1. one plugin per channel: Two ChannelData managers, growing in opposite directions within the same pre-reserved
 *         memory block.
 *  2. stereo plugin on both channels: Single ChannelData manager, occupying the entire pre-reserved memory block.
 *
 *  memory_block: continuous compile time allocated memory, of size TBD_SOUND_PROCESSOR_MAX_MEMORY
 *  start: first byte of memory_block
 *  end: last byte of memory block
 *
 *  one plugin per channel case:
 *
 *  +------+ start
 *  | cls0 |
 *  +------+ start + sizeof(cls0)
 *  | buf0 |
 *  +------+ start + ch_0_size
 *  | ...  |
 *  +------+ end - ch_1_size
 *  | buf1 |
 *  +------+ end - sizeof(cls1)
 *  | cls1 |
 *  +------+ end
 *
 *  stereo plugin case:
 *
 *  +------+ start
 *  | cls0 |
 *  +------+ start + sizeof(cls0)
 *  | buf0 |
 *  +------+ start + ch_0_size
 *  | ...  |
 *  +------+ end
 *
 */
struct ChannelData {
    ChannelData(uint8_t* start, uint8_t* end) : start_(start), end_(end), top_(end) {}

    [[nodiscard]] PluginMetaBase* get_processor() const { return processor_; }

    size_t reset() {
        const size_t freed_bytes = buffer_pos_ == nullptr ? 0 : std::abs(start_ - buffer_pos_);
        if (!is_empty()) {
            processor_->~PluginMetaBase();
        }
        processor_ = nullptr;
        buffer_base_ = nullptr;
        buffer_pos_ = nullptr;
        return freed_bytes;
    }

    Error set_processor(PluginMetaBase* processor) {
        if (processor == nullptr) {
            return TBD_ERR(SOUND_REGISTRY_BAD_SOUND_PROCESSOR);
        }
        if (processor != processor_address()) {
            return TBD_ERR(SOUND_REGISTRY_BAD_SOUND_PROCESSOR_ADDRESS);
        }
        if (!is_empty()) {
            return TBD_ERR(SOUND_REGISTRY_CHANNEL_IN_USE);
        }
        if (buffer_base_ == nullptr) {
            return TBD_ERR(SOUND_REGISTRY_CHANNEL_NOT_RESERVED);
        }
        processor_ = processor;
        return TBD_OK;
    }

    Maybe<void*> reserve_plugin_memory(const size_t size) {
        if (!is_empty()) {
            return TBD_FAIL(SOUND_REGISTRY_CHANNEL_IN_USE);
        }

        if (total_available_size() < size) {
            return TBD_FAIL(SOUND_REGISTRY_CHANNEL_IN_USE);
        }

        buffer_base_ = start_ + dir() * size;
        buffer_pos_ = buffer_base_;
        if (dir() < 0) {
            return {buffer_base_ + 1};
        }
        return {start_};
    }

    void grow(const size_t size) {
        top_ += dir() * size;
    }

    void shrink(const size_t size) {
        top_ -= dir() * size;
    }

    [[nodiscard]] bool is_empty() const {
        return processor_ == nullptr;
    }

    [[nodiscard]] size_t size() const {
        return (buffer_pos_ - start_) * dir();
    }

    [[nodiscard]] size_t total_available_size() const {
        return (top_ - start_) * dir();
    }

private:
    [[nodiscard]] int dir() const {
        return end_ - start_ > 0 ? 1 : -1;
    }

    [[nodiscard]] PluginMetaBase* processor_address() const {
        uint8_t* address = dir() < 0 ? buffer_base_ + 1 : start_;
        return reinterpret_cast<PluginMetaBase*>(address);
    }

    // first byte in buffer
    uint8_t* const start_;
    // first byte past buffer
    uint8_t* const end_;

    // memory block at start of memory buffer holding sound processor class
    PluginMetaBase* processor_ = nullptr;
    // first byte after sound processor block
    uint8_t* buffer_base_ = nullptr;
    // next available byte after sound processor memory and acquired buffers
    uint8_t* buffer_pos_ = nullptr;
    // first byte past reservable memory
    uint8_t* top_;
};

struct ActiveSoundProcessorsImpl {
    ActiveSoundProcessorsImpl()
        : left_(memory_block_, memory_block_ + TOTAL_SIZE),
          right_(memory_block_ + (TOTAL_SIZE - 1), memory_block_ - 1)
    {}

    [[nodiscard]] bool is_stereo() const { return is_stereo_; }
    [[nodiscard]] Maybe<PluginMetaBase*> get_processor_on_channels(const Channels channels) const {
        if (channels & CM_LEFT) {
            if (is_stereo() && channels != CM_BOTH) {
                return TBD_FAIL(SOUND_REGISTRY_NEED_STEREO);
            }
            if (const auto processor = left_.get_processor(); processor != nullptr) {
                return {processor};
            }
            return TBD_FAIL(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
        }
        if (channels & CM_RIGHT) {
            if (const auto processor = right_.get_processor(); processor != nullptr) {
                return {processor};
            }
            return TBD_FAIL(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
        }
        return TBD_FAIL(SOUND_REGISTRY_BAD_CHANNEL_MAPPING);
    }
    [[nodiscard]] PluginMetaBase* on_right() const { return right_.get_processor(); }

    Maybe<void*> reserve_plugin_memory(const Channels channels, const size_t plugin_size) {
        const Channels to_reset = is_stereo_ ? CM_BOTH : channels;
        if (to_reset & CM_LEFT) {
            right_.grow(left_.reset());
        }
        if (to_reset & CM_RIGHT) {
            left_.grow(right_.reset());
        }

        is_stereo_ = channels == CM_BOTH;
        if (is_stereo_ || channels == CM_LEFT) {
            const auto result = left_.reserve_plugin_memory(plugin_size);
            if (result) {
                right_.shrink(plugin_size);
            }
            return result;
        }
        if (channels == CM_RIGHT) {
            const auto result =  right_.reserve_plugin_memory(plugin_size);
            if (result) {
                left_.shrink(plugin_size);
            }
            return result;
        }
        return TBD_FAIL(SOUND_REGISTRY_BAD_CHANNEL_MAPPING);
    }

    Error set_active_plugin(const Channels channels, PluginMetaBase* plugin) {
        if (channels == CM_BOTH && (!right_.is_empty() || !left_.is_empty())) {
            return TBD_ERR(SOUND_REGISTRY_CHANNEL_IN_USE);
        }
        if (channels == CM_LEFT || channels == CM_BOTH) {
            return left_.set_processor(plugin);
        }
        if (channels == CM_RIGHT) {
            return right_.set_processor(plugin);
        }
        return TBD_ERR(SOUND_REGISTRY_BAD_CHANNEL_MAPPING);
    }

    void reset() {
        right_.grow(left_.reset());
        left_.grow(right_.reset());
    }

    [[nodiscard]] size_t remaining_buffer_size() const {
        return TOTAL_SIZE - (left_.size() + right_.size());
    }

private:
    uint8_t memory_block_[TOTAL_SIZE];
    bool is_stereo_ = false;
    ChannelData left_;
    ChannelData right_;

} impl;

}

bool ActiveSoundProcessors::is_stereo() {
    return impl.is_stereo();
}

Maybe<PluginID> ActiveSoundProcessors::get_processor_on_channels(Channels channels) {
    const auto processor = impl.get_processor_on_channels(channels);
    if (!processor) {
        return {processor.error()};
    }
    return {processor->id()};
}

Error ActiveSoundProcessors::set_param(const Channels channels, const ParameterID param_id, const int_par value) {
    auto processor = impl.get_processor_on_channels(channels);
    if (!processor) {
        return {processor.error()};
    }
    if (const auto err = processor->set_param(param_id, value); err != TBD_OK) {
        return err;
    }
    sound_processor_param_changed(channels, param_id, value);
    return TBD_OK;
}

Error ActiveSoundProcessors::set_param(const Channels channels, const ParameterID param_id, const uint_par value) {
    auto processor = impl.get_processor_on_channels(channels);
    if (!processor) {
        return {processor.error()};
    }
    if (const auto err = processor->set_param(param_id, value); err != TBD_OK) {
        return err;
    }
    sound_processor_param_changed(channels, param_id, value);
    return TBD_OK;
}

Error ActiveSoundProcessors::set_param(const Channels channels, const ParameterID param_id, const float_par value) {
    auto processor = impl.get_processor_on_channels(channels);
    if (!processor) {
        return {processor.error()};
    }
    const uint_par value_data = *reinterpret_cast<const uint32_t*>(&value);
    if (const auto err = processor->set_param(param_id, value_data); err != TBD_OK) {
        return err;
    }
    sound_processor_param_changed(channels, param_id, value_data);
    return TBD_OK;
}

Error ActiveSoundProcessors::set_param(const Channels channels, const ParameterID param_id, const trigger_par value) {
    auto processor = impl.get_processor_on_channels(channels);
    if (!processor) {
        return {processor.error()};
    }
    if (const auto err = processor->set_param(param_id, value); err != TBD_OK) {
        return err;
    }
    sound_processor_param_changed(channels, param_id, value);
    return TBD_OK;
}

Error ActiveSoundProcessors::map_param(const Channels channels, const ParameterID param_id, const InputID input_id) {
    auto processor = impl.get_processor_on_channels(channels);
    if (!processor) {
        return {processor.error()};
    }
    if (const auto err = processor->map_param(param_id, input_id); err != TBD_OK) {
        return err;
    }
    sound_processor_mapping_changed(channels, param_id, input_id);
    return TBD_OK;
}

Maybe<void*> ActiveSoundProcessors::reserve_plugin_memory(const Channels channels, const size_t plugin_size) {
    return impl.reserve_plugin_memory(channels, plugin_size);
}

Error ActiveSoundProcessors::set_active_plugin(const Channels channels, PluginMetaBase* plugin) {
    return impl.set_active_plugin(channels, plugin);
}

void ActiveSoundProcessors::reset() {
    impl.reset();
    sound_processors_reset();
}

size_t ActiveSoundProcessors::remaining_buffer_size() {
    return impl.remaining_buffer_size();
}

}
