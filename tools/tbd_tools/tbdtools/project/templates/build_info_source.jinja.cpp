#include <tbd/version.hpp>


namespace CTAG {

const std::string hardware_type("{{ build_info.hardware }}");
const std::string firmware_version("{{ build_info.firmware }}");

const std::string commit("{{ build_info.commit }}");
const bool post_commit_changes = {{ 'true' if build_info.post_commit_changes else 'false' }};
const uint8_t ahead_of_last_release = {{ build_info.ahead_of_release }};
const std::string build_date("{{ build_info.build_date }}");

}
