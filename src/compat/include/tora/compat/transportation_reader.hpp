#pragma once
#include <tora/transportation/north_west.hpp>
#include <string_view>
namespace tora::compat {
// Read-only support for the observed default-format Transportation text file.
transportation::Problem readTransportation(std::string_view bytes);
}
