#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue {

namespace text_processing {

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);


}

}