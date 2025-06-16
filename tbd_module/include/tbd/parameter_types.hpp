/** TBD reflection primitives.
 *
 *  This file defines the valid TBD reflectable parameter primitives. Reflectable primitives are restricted to a
 *  well defined aliased subset of scalar types, to distinguish them from ordinary scalars. Using these explicitly
 *  states that a function/class/struct is reflectable and prevents any unintentional serialization/processing by the
 *  reflection system.
 *
 *  Purpose and Use Cases of Reflectables
 *  -------------------------------------
 *
 *  The two use cases of reflectable types are as
 *
 *  1. function arguments (reflectable functions)
 *  2. fields on classes/structs (reflectable classes)
 *
 *  Use cases of reflectable functions include the `tbd_api` for endpoints and events, where they are used in
 *  conjunction with the `tbd::endpoint` decorator to create API handler wrappers. Reflectable classes/struct are
 *  generally used to declare reflectable data types for serialization/transmission as well as for specific use cases
 *  such as:
 *
 *  - sound generator plugins (declaring input parameters and parameter transforms)
 *  - event system (declaring message types using `tbd::message` decorator`)
 *
 *  Important Differences to Reflection in other Languages/Systems
 *  --------------------------------------------------------------
 *
 *  There are some important differences between TBD reflection and comparable solutions in languages like Java,
 *  Python, etc.:
 *
 *  1. TBD reflection is passive: Reflectable types are processed by specific TBD modules for specific purposes. Files
 *         are not explicitly registered for reflection by the providing module and/or the module evaluating specific
 *         decorators is present in the build config, annotations will simply be ignored, while the defined plain
 *         types/functions can still be used as if unannotated.
 *  2. TBD reflection is not intrusive: When declaring annotated functions or types, the original name will still refer
 *         refer to that plain function/type. Unlike Python decorators for example, wrappers do not hide the original
 *         entity and have their own domain specific names.
 */
#pragma once

#include <string>
#include <cinttypes>

namespace tbd {
    using int_par     = int32_t;
    using uint_par    = uint32_t;
    using float_par   = float;
    using ufloat_par  = float;
    using trigger_par = bool;
    using str_par     = std::string;
};
