#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>

using namespace std;
using namespace transport_catalogue;

int main() {
    database::TransportCatalogue catalogue;
    filling::InputReader reader;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        retrieving::ParseAndPrintStat(catalogue, line, cout);
    }
}