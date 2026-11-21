#pragma once
#include <tora/compat/lp_file.hpp>
#include <tora/compat/transportation_document.hpp>
#include "games_document.hpp"
#include <QByteArray>
namespace tora::desktop {
compat::LpDocument decodeLpJson(const QByteArray& bytes);
QByteArray encodeLpJson(const compat::LpDocument& document);
compat::TransportationDocument decodeTransportationJson(const QByteArray& bytes);
QByteArray encodeTransportationJson(const compat::TransportationDocument& document);
GamesDocument decodeGameJson(const QByteArray& bytes);
QByteArray encodeGameJson(const GamesDocument& document);
}
