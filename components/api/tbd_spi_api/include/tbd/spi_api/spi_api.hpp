#pragma once

#include <tbd/errors.hpp>

namespace tbd::spi_api {

Error begin();
Error end();
void do_work();

}