#include <tbd/drivers/common/codec.hpp>

#include <string>


namespace tbd::drivers::file_codec {

void set_source(const std::string& source_file);

void set_sink(const std::string& sink_file);

}