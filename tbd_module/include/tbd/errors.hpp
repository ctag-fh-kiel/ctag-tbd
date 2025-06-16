#pragma once

#include <cstddef>
#include <cinttypes>

namespace tbd { using Error = uint32_t; }

namespace tbd::errors {

size_t get_num_errors();
const char* get_error_name(Error err);
const char* get_error_message(Error err);

}

#define TBD_OK ::tbd::errors::SUCCESS

#if USE_TBD_ERR
#include <tbd/errors/all_errors.hpp>
#else

/**
 *  @brief declare a reusable error
 */
#define TBD_NEW_ERR(err, msg)

/**
 *  @brief get reusable error code by name
 */
#define TBD_ERR(err) ::tbd::errors::FAILURE

/**
 *  @brief get single use error code
 */
#define TBD_ERRMSG(err, msg) ::tbd::errors::FAILURE

namespace tbd::errors {

enum Errors {
    SUCCESS = 0,
    FAILURE = 1,
};

inline size_t get_num_errors() {
    return 2;
}

inline const char* get_error_name(Error err) {
    return "FAILURE";
}

inline const char* get_error_message(Error err) {
    return "unknown error";
}

}

#endif
