#pragma once

#include <tbd/errors.hpp>

namespace tbd::spi_inputs {

Error begin();
Error end();
void do_work();

}
