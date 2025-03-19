#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace transport_catalogue {

namespace text_processing {
 
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(',', not_space2);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    const auto count = string.find_last_not_of(' ') + 1 - start;
    return string.substr(start, count);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
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
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

std::unordered_map<std::string, int> ParseDistances(std::string_view string) {
    auto str = std::string_view(string.substr(string.find(',') + 1));
    auto comma2 = str.find(',');
    auto s = str.substr(comma2 + 1);
    auto distance_to_name = Split(s, ',');
    std::unordered_map<std::string, int> result;
    for (const auto& line : distance_to_name) {
        auto not_space = line.find_first_not_of(' ');
        auto m_pos = line.find('m');
        int d = std::stoi(std::string(line.substr(not_space, m_pos - not_space)));

        std::string name = std::string(Trim(line.substr(line.find('o') + 1)));

        result.insert({std::move(name), d});
    }
    return result;
} 

CommandDescription ParseCommandDescription(std::string_view line) {
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

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    using namespace std::literals;
    if (commands_.empty()) {
        return;
    }
    for (const auto& c : commands_) {
        if (c.command == "Stop"s) {
            catalogue.AddStop(Stop{std::move(c.id), std::move(ParseCoordinates(Trim(c.description)))});
        }
    }
    for (const auto& c : commands_) {
        if (c.command == "Stop"s) {
            catalogue.AddDistances(std::move(c.id), ParseDistances(c.description));
        }
        if (c.command == "Bus"s) {
            std::vector<const Stop*> busroute;
            for (const auto& stop : ParseRoute(Trim(c.description))) {
                busroute.push_back(catalogue.GetStop(stop));
            }
            catalogue.AddBus(Bus{std::move(c.id), std::move(busroute)});
        }
    }
}

}

}