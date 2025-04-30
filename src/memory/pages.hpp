#pragma once

#include <cstddef>
#include <sys/mman.h>
#include <unistd.h>

namespace strobe {

std::size_t page_size();

}
