#pragma once

#include <tbd/parameter_types.hpp>


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
    SUCCESS    = 0,
    FAILURE    = 1,
    NUM_ERRORS = 2,
};

inline uint16_t get_num_errors() {
    return NUM_ERRORS;
}

inline const char* get_error_name(const Errors err) {
    return "FAILURE";
}

inline const char* get_error_message(const Errors err) {
    return "unknown error";
}

inline Errors err_from_int(const uint_par err) {
    return err == SUCCESS ? SUCCESS : FAILURE;
}

}

#endif

namespace tbd { using Error = errors::Errors; }

namespace tbd::errors {

inline const char* get_error_name(const uint_par err) {
    return get_error_name(err_from_int(err));
}

inline const char* get_error_message(const uint_par err) {
    return get_error_message(err_from_int(err));
}

}