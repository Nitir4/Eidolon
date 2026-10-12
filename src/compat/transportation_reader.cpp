#include <tora/compat/transportation_reader.hpp>
#include <tora/compat/transportation_document.hpp>

namespace tora::compat {
transportation::Problem readTransportation(std::string_view bytes) {
    return TransportationDocument::decode(bytes).problem();
}
}
