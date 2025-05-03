#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace std;
using namespace transport_catalogue;

int main() {
    try {
        auto document = json::Load(cin);
    
        database::TransportCatalogue catalogue;
        processing::JsonReader reader(catalogue);

        reader.ProcessDocument(document, cout);
        
    } catch (const json::ParsingError& e) {
        cerr << "Error parsing input file: "s << e.what() << endl;   
    } catch (const exception& e) {
        cerr << "Exception: "s << e.what() << endl;
    }
    return 0;
}