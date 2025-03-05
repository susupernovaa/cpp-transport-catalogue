#include <iostream>
#include <string>
#include <fstream>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue catalogue;
    std::ifstream input;
    input.open("input.txt");
    int base_request_count;
    input >> base_request_count >> ws;

    {
        text_processing::InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(input, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    input >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        text_processing::ParseAndPrintStat(catalogue, line, cout);
    }
}