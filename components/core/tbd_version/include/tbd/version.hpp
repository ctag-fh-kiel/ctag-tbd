#include <string>
#include <cstdint>


namespace tbd::sysinfo {

extern const std::string hardware_type;
extern const std::string firmware_version;
extern const std::string device_capabilities;

extern const std::string commit;
extern const bool post_commit_changes;
extern const uint8_t ahead_of_last_release;
extern const std::string build_date;

}
