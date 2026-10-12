#pragma once
#include <tora/transportation/north_west.hpp>
#include <string_view>
namespace tora::compat {
// Convenience numerical-model view of the default-format document reader.
transportation::Problem readTransportation(std::string_view bytes);
}
