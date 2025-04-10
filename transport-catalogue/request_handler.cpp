#include "map_renderer.h"

namespace transport_catalogue {

namespace renderer {

MapRenderer::MapRenderer(RenderSettings settings, std::vector<geo::Coordinates> coords) 
    : settings_(settings), proj_(SetProjector(coords)) {
}

const svg::Document* MapRenderer::RenderRoutes(const std::deque<Bus>& buses, const std::deque<Stop>& stops) {
    size_t i = 0;
    for (const auto& bus : buses) {
        if (bus.busroute.empty()) {
            continue;
        }
        svg::Color color = settings_.color_palette[i % settings_.color_palette.size()];
        AddRoute(bus, color);
        ++i;
    }
    i = 0;
    for (const auto& bus : buses) {
        if (bus.busroute.empty()) {
            continue;
        }
        svg::Color color = settings_.color_palette[i % settings_.color_palette.size()];
        AddBusnames(bus, color);
        ++i;
    }
    for (const auto& stop : stops) {
        AddStop(stop);
    }
    for (const auto& stop : stops) {
        AddStopname(stop);
    }
    return &map_;
}

SphereProjector MapRenderer::SetProjector(std::vector<geo::Coordinates> coords) {
    return SphereProjector{coords.begin(), coords.end(), 
        settings_.width, settings_.height, settings_.padding};
}

void MapRenderer::AddRoute(const Bus& bus, const svg::Color& color) {
    using namespace svg;
    Polyline route;
    route.SetFillColor("none").SetStrokeColor(color).SetStrokeWidth(settings_.line_width);
    route.SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);
    for (const Stop* stop : bus.busroute) {
        route.AddPoint(proj_(stop->coordinates));
    }
    map_.Add(std::move(route));
}

svg::Text MapRenderer::RenderBusname(const std::string busname, const Stop* stop, const svg::Color& color) const {
    svg::Text result;
    result.SetPosition(proj_(stop->coordinates)).SetOffset(settings_.bus_label_offset);
    result.SetFontSize(settings_.bus_label_font_size).SetFontFamily(settings_.font_family);
    result.SetFontWeight(settings_.font_weight).SetData(busname).SetFillColor(color);
    return result;
}

svg::Text MapRenderer::RenderBusnameUnderlayer(const std::string& busname, const Stop* stop) const {
    using namespace svg;
    Text result = RenderBusname(busname, stop);
    result.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color);
    result.SetStrokeWidth(settings_.underlayer_width);
    result.SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);
    return result;
}

void MapRenderer::AddBusnames(const Bus& bus, const svg::Color& color) {
    using namespace svg;
    const Stop* stop = bus.busroute.front();
    map_.Add(RenderBusnameUnderlayer(bus.busname, stop));
    map_.Add(RenderBusname(bus.busname, stop, color));
    if (!bus.is_roundtrip && bus.busroute.at(bus.busroute.size() / 2)->stopname != 
        bus.busroute.front()->stopname) {
            const Stop* stop = bus.busroute.at(bus.busroute.size() / 2);
            map_.Add(RenderBusnameUnderlayer(bus.busname, stop));
            map_.Add(RenderBusname(bus.busname, stop, color));
    }
}

void MapRenderer::AddStop(const Stop& stop) {
    using namespace svg;
    Circle circle;
    circle.SetCenter(proj_(stop.coordinates)).SetRadius(settings_.stop_radius).SetFillColor("white");
    map_.Add(circle);
}

svg::Text MapRenderer::RenderStopname(const Stop& stop) const {
    svg::Text result;
    result.SetPosition(proj_(stop.coordinates)).SetOffset(settings_.stop_label_offset);
    result.SetFontSize(settings_.stop_label_font_size).SetFontFamily(settings_.font_family);
    result.SetData(stop.stopname).SetFillColor("black");
    return result;
}

svg::Text MapRenderer::RenderStopnameUnderlayer(const Stop& stop) const {
    using namespace svg;
    Text result = RenderStopname(stop);
    result.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color);
    result.SetStrokeWidth(settings_.underlayer_width);
    result.SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);
    return result;
}

void MapRenderer::AddStopname(const Stop& stop) {
    using namespace svg;
    map_.Add(RenderStopnameUnderlayer(stop));
    map_.Add(RenderStopname(stop));
}

} // namespace renderer

} // namespace transport_catalogue