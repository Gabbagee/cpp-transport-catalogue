#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace std;
using namespace transport_catalogue;

int main() {
    database::TransportCatalogue catalogue;
    filling::InputReader reader;

    reader.ReadBaseRequests(catalogue, cin);
    retrieving::ReadStatRequests(catalogue, cin, cout);
}