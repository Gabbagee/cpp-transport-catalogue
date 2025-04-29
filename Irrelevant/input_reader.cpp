#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

using namespace std;

namespace transport_catalogue::filling {
    
using namespace transport_catalogue::geo;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = stod(string(str.substr(not_space, comma - not_space)));
    double lng = stod(string(str.substr(not_space2)));

    return {lat, lng};
}

vector<pair<string_view, int>> ParseDistances(string_view string) {
    vector<pair<string_view, int>> dists;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto unit_pos = string.find("m to "s, pos);
        int dist = stoi(std::string(string.substr(pos, unit_pos - pos)));
        auto stop_pos = unit_pos + 5;
        auto delim_pos = string.find(',', stop_pos);
        
        if (delim_pos == string_view::npos) {
            delim_pos = string.size();
        }

        auto stop_name = string.substr(stop_pos, delim_pos - stop_pos);
        dists.emplace_back(stop_name, dist);

        pos = delim_pos + 1;
    }

    return dists;
}

/**
 * Удаляет пробелы в начале и конце строки
 */
string_view Trim(string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
vector<string_view> Split(string_view string, char delim) {
    vector<string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string_view::npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
vector<string_view> ParseRoute(string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    vector<string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {string(line.substr(0, space_pos)),
            string(line.substr(not_space, colon_pos - not_space)),
            string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for (const auto& command : commands_) {
        if (command.command == "Stop"s) {
            auto coords = ParseCoordinates(command.description.substr(0, command.description.find(',', command.description.find(',') + 1)));
            catalogue.AddStop(command.id, coords);
        }
    }

    for (const auto& command : commands_) {
        if (command.command == "Stop"s) {            
            size_t first_comma_pos = command.description.find(',');
            size_t second_comma_pos = command.description.find(',', first_comma_pos + 1);
            auto dists_raw = ""s;

            if (second_comma_pos != string_view::npos) {
                dists_raw = command.description.substr(second_comma_pos + 1);
            }
            
            auto dists = ParseDistances(dists_raw);
            const Stop* stop_info = catalogue.GetStopInfo(command.id).value();

            for (const auto& [dest_name, dist] : dists) {
                const auto* dest_info = catalogue.GetStopInfo(dest_name).value();
                catalogue.SetDistance(stop_info, dest_info, dist);

                if (catalogue.GetDistance(dest_info, stop_info) == 0) {
                    catalogue.SetDistance(dest_info, stop_info, dist);
                }
            }
        }
    }

    for (const auto& command : commands_) {
        if (command.command == "Bus"s) {
            auto stops = ParseRoute(command.description);
            bool is_circular = command.description.find('>') != string::npos;
            catalogue.AddRoute(command.id, stops, is_circular);
        }
    }
}

void InputReader::ReadBaseRequests(TransportCatalogue& catalogue, istream& input) {
    int base_request_count;
    input >> base_request_count >> ws;

    for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(input, line);
            ParseLine(line);
        }
    ApplyCommands(catalogue);
}

}  // namespace transport_catalogue::filling