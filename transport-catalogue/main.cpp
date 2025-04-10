#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <fstream>
#include <iostream>

int main() {
    using namespace transport_catalogue;
    json_processing::JsonReader reader;
    TransportCatalogue catalogue;
    reader.ParseInput(std::cin);
    reader.FillBase(catalogue);
    renderer::MapRenderer renderer(reader.GetRenderSettings(), catalogue.GetAllCoordinates());
    RequestHandler handler{catalogue, renderer};
    json_processing::JsonPrinter printer{handler, reader.GetStatRequests()};
    printer.PrintStats(std::cout);
}