/*
 * railway_handler_pass1.cpp
 *
 *  Created on:  2017-08-21
 *      Author: Michael Reichert <michael.reichert@geofabrik.de>
 */

#include "railway_handler_pass1.hpp"
#include <iostream>

/// indexes of fields – all layers
struct FieldIndexes {
    static constexpr int node_id = 0;
    static constexpr int way_id = 0;
    static constexpr int lastchange = 1;
};

/// crossings layer
struct CrossingIndexes {
    static constexpr int barrier = 2;
    static constexpr int lights = 3;
};

/// additional fields of the layers for stops, platforms and stations
struct StopsPlatformsStationIndexes {
    static constexpr int name = 2;
    static constexpr int public_transport = 3;
    static constexpr int railway = 4;
    static constexpr int highway = 5;
    static constexpr int _operator = 6;
    static constexpr int network = 7;
};

/// additional fields of the layers for stops and platforms
struct StopsPlatformsIndexes {
    static constexpr int ref = 8;
    static constexpr int local_ref = 9;
};

/// additional field of the station layer
struct StationsIndexes {
    static constexpr int amenity = 8;
};

RailwayHandlerPass1::RailwayHandlerPass1(OGRWriter& writer, Options& options,
        osmium::util::VerboseOutput& verbose_output, osmium::ItemStash& signals,
        std::unordered_map<osmium::object_id_type, osmium::ItemStash::handle_type>& must_on_track_handles) :
        m_output(writer, verbose_output, options),
        m_must_on_track(signals),
        m_must_on_track_handles(must_on_track_handles) {
    if (options.crossings) {
        m_crossings = m_output.writer().create_layer_ptr("crossings", wkbPoint);
        // add fields to layers
        m_crossings->add_field("node_id", OFTString, 10);
        m_crossings->add_field("lastchange", OFTString, 21);
        m_crossings->add_field("barrier", OFTString, 50);
        m_crossings->add_field("lights", OFTString, 50);
    }
    if (options.stops) {
        m_stops = m_output.writer().create_layer_ptr("stops", wkbPoint);
        // add fields to layers
        m_stops->add_field("node_id", OFTString, 10);
        m_stops->add_field("lastchange", OFTString, 21);
        m_stops->add_field("name", OFTString, 100);
        m_stops->add_field("public_transport", OFTString, 50);
        m_stops->add_field("railway", OFTString, 50);
        m_stops->add_field("highway", OFTString, 50);
        m_stops->add_field("operator", OFTString, 100);
        m_stops->add_field("network", OFTString, 100);
        m_stops->add_field("ref", OFTString, 50);
        m_stops->add_field("local_ref", OFTString, 50);
    }
    if (options.platforms) {
        m_platforms = m_output.writer().create_layer_ptr("platforms", wkbPoint);
        // add fields to layers
        m_platforms->add_field("node_id", OFTString, 10);
        m_platforms->add_field("lastchange", OFTString, 21);
        m_platforms->add_field("name", OFTString, 100);
        m_platforms->add_field("public_transport", OFTString, 50);
        m_platforms->add_field("railway", OFTString, 50);
        m_platforms->add_field("highway", OFTString, 50);
        m_platforms->add_field("operator", OFTString, 100);
        m_platforms->add_field("network", OFTString, 100);
        m_platforms->add_field("ref", OFTString, 25);
        m_platforms->add_field("local_ref", OFTString, 25);
        m_platforms_l = m_output.writer().create_layer_ptr("platforms_l", wkbLineString);
        m_platforms_l->add_field("way_id", OFTString, 10);
        m_platforms_l->add_field("lastchange", OFTString, 21);
        m_platforms_l->add_field("name", OFTString, 21);
        m_platforms_l->add_field("public_transport", OFTString, 50);
        m_platforms_l->add_field("railway", OFTString, 50);
        m_platforms_l->add_field("highway", OFTString, 50);
        m_platforms_l->add_field("operator", OFTString, 100);
        m_platforms_l->add_field("network", OFTString, 100);
        m_platforms_l->add_field("ref", OFTString, 25);
        m_platforms_l->add_field("local_ref", OFTString, 25);
    }
    if (options.stations) {
        m_stations = m_output.writer().create_layer_ptr("stations", wkbPoint);
        // add fields to layers
        m_stations->add_field("node_id", OFTString, 10);
        m_stations->add_field("lastchange", OFTString, 21);
        m_stations->add_field("name", OFTString, 100);
        m_stations->add_field("public_transport", OFTString, 50);
        m_stations->add_field("railway", OFTString, 50);
        m_stations->add_field("highway", OFTString, 50);
        m_stations->add_field("operator", OFTString, 100);
        m_stations->add_field("network", OFTString, 100);
        m_stations->add_field("amenity", OFTString, 50);
        m_stations_l = m_output.writer().create_layer_ptr("stations_l", wkbLineString);
        m_stations_l->add_field("way_id", OFTString, 10);
        m_stations_l->add_field("lastchange", OFTString, 21);
        m_stations_l->add_field("name", OFTString, 100);
        m_stations_l->add_field("public_transport", OFTString, 50);
        m_stations_l->add_field("railway", OFTString, 50);
        m_stations_l->add_field("highway", OFTString, 50);
        m_stations_l->add_field("operator", OFTString, 100);
        m_stations_l->add_field("network", OFTString, 100);
        m_stations_l->add_field("amenity", OFTString, 50);
    }
    if (options.stops || options.platforms) {
        m_stops_only_highway = m_output.writer().create_layer_ptr("stops_only_highway", wkbPoint);
        // add fields to layers
        m_stops_only_highway->add_field("node_id", OFTString, 10);
        m_stops_only_highway->add_field("lastchange", OFTString, 21);
        m_stops_only_highway->add_field("name", OFTString, 100);
        m_stops_only_highway->add_field("public_transport", OFTString, 50);
        m_stops_only_highway->add_field("railway", OFTString, 50);
        m_stops_only_highway->add_field("highway", OFTString, 50);
        m_stops_only_highway->add_field("operator", OFTString, 100);
        m_stops_only_highway->add_field("network", OFTString, 100);
    }
}

void RailwayHandlerPass1::node(const osmium::Node& node) {
    if (!node.location().valid()) {
        return;
    }
    const char* railway = node.get_value_by_key("railway");
    const char* public_transport = node.get_value_by_key("public_transport");
    if (m_output.options().railway_details && railway &&
            (!strcmp(railway, "signal") || !strcmp(railway, "stop")
             || !strcmp(railway, "buffer_stop") || !strcmp(railway, "level_crossing")
             || !strcmp(railway, "milestone") || !strcmp(railway, "derail")
             || !strcmp(railway, "isolated_track_section") || !strcmp(railway, "switch")
             || !strcmp(railway, "railway_crossing"))) {
        m_must_on_track_handles.emplace(node.id(), m_must_on_track.add_item(node));
    } else if (public_transport && !strcmp(public_transport, "stop_position")) {
        m_must_on_track_handles.emplace(node.id(), m_must_on_track.add_item(node));
    }
    handle_stop(node, public_transport, railway);
    if (railway && m_output.options().crossings && (!strcmp(railway, "level_crossing") || !strcmp(railway, "crossing"))) {
        handle_crossing(node);
    }
}

void RailwayHandlerPass1::handle_crossing(const osmium::Node& node) {
    assert(m_output.options().crossings);
    const char* barrier = node.get_value_by_key("crossing:barrier");
    const char* lights = node.get_value_by_key("crossing:light");
    std::string barrier_value;
    std::string lights_value;
    if (!barrier) {
        barrier_value = "NONE";
    } else if (barrier && strcmp(barrier, "no") && strcmp(barrier, "yes")
            && strcmp(barrier, "half") && strcmp(barrier, "double_half")
            && strcmp(barrier, "full") && strcmp(barrier, "gates")) {
        barrier_value = "UNKNOWN";
    } else {
        barrier_value = barrier;
    }
    if (!lights) {
        lights_value = "NONE";
    } else if (lights && strcmp(lights, "yes") && strcmp(lights, "no")) {
        lights_value = "UNKNOWN";
    } else {
        lights_value = lights;
    }
    add_crossing_node(node, CrossingIndexes::barrier, barrier_value.c_str(), CrossingIndexes::lights, lights_value.c_str());
}

void RailwayHandlerPass1::handle_stop(const osmium::OSMObject& object, const char* public_transport, const char* railway) {
    if (m_output.options().stations) {
        if ((public_transport && !strcmp(public_transport, "station"))
                || (railway && (!strcmp(railway, "station") || !strcmp(railway, "halt")
                        || !strcmp(railway, "tram_stop") || object.tags().has_tag("amenity", "bus_station")))) {
            switch (object.type()) {
            case osmium::item_type::node :
                add_stop_pltf_node(*m_stations, static_cast<const osmium::Node&>(object), false, true);
                break;
            case osmium::item_type::way :
                add_stop_pltf_way(*m_stations_l, static_cast<const osmium::Way&>(object), false, true);
                break;
            default:
                break;
            }
            return;
        }
    }
    if (m_output.options().platforms) {
        if ((public_transport && !strcmp(public_transport, "platform")) || (railway && !strcmp(railway, "platform"))) {
            switch (object.type()) {
            case osmium::item_type::node :
                add_stop_pltf_node(*m_platforms, static_cast<const osmium::Node&>(object), true, false);
                break;
            case osmium::item_type::way :
                add_stop_pltf_way(*m_platforms_l, static_cast<const osmium::Way&>(object), true, false);
                break;
            default:
                break;
            }
            return;
        }
    }
    if (object.type() != osmium::item_type::node) {
        return;
    }
    if (m_output.options().stops && public_transport && !strcmp(public_transport, "stop_position")) {
        add_stop_pltf_node(*m_stops, static_cast<const osmium::Node&>(object), true, false);
        return;
    }
    if ((m_output.options().stops || m_output.options().platforms) && object.tags().has_tag("highway", "bus_stop") && !object.tags().has_key("public_transport")) {
        add_stop_pltf_node(*m_stops_only_highway, static_cast<const osmium::Node&>(object), false, false);
    }
}

void RailwayHandlerPass1::way(const osmium::Way& way) {
    try {
        if (m_output.options().stops || m_output.options().stations || m_output.options().platforms) {
            const char* railway = way.get_value_by_key("railway");
            const char* public_transport = way.get_value_by_key("public_transport");
            handle_stop(way, public_transport, railway);
        }
    } catch (osmium::invalid_location& err) {
        m_output.verbose_output() << err.what() << '\n';
    }
}

void RailwayHandlerPass1::add_crossing_node(const osmium::Node& node, const int third_field_index,
        const char* third_field_value, const int fourth_field_index, const char* fourth_field_value) {
    if (!m_output.coordinates_valid(node)) {
        return;
    }
    gdalcpp::Feature feature(*m_crossings, m_output.factory().create_point(node));
    set_node_id(feature, node);
    std::string the_timestamp (node.timestamp().to_iso());
    feature.set_field(FieldIndexes::lastchange, the_timestamp.c_str());
    if (third_field_value) {
        feature.set_field(third_field_index, third_field_value);
    }
    if (fourth_field_value) {
        feature.set_field(fourth_field_index, fourth_field_value);
    }
    feature.add_to_layer();
}

void RailwayHandlerPass1::add_stop_pltf_node(gdalcpp::Layer& layer, const osmium::Node& node, bool refs, bool amenity) {
    if (!m_output.coordinates_valid(node)) {
        return;
    }
    gdalcpp::Feature feature(layer, m_output.factory().create_point(node));
    set_node_id(feature, node);
    set_fields(feature, node, refs, amenity);
    feature.add_to_layer();
}

void RailwayHandlerPass1::add_stop_pltf_way(gdalcpp::Layer& layer, const osmium::Way& way, bool refs, bool amenity) {
    if (!m_output.coordinates_valid(way)) {
        return;
    }
    try {
        gdalcpp::Feature feature(layer, m_output.factory().create_linestring(way));
        set_way_id(feature, way);
        set_fields(feature, way, refs, amenity);
        feature.add_to_layer();
    } catch (osmium::geometry_error& err) {
        m_output.verbose_output() << err.what() << '\n';
    }
}

/*static*/ void RailwayHandlerPass1::set_fields(gdalcpp::Feature& feature, const osmium::OSMObject& object,
        bool refs, bool amenity) {
    std::string the_timestamp (object.timestamp().to_iso());
    feature.set_field(FieldIndexes::lastchange, the_timestamp.c_str());
    feature.set_field(StopsPlatformsStationIndexes::railway, object.get_value_by_key("railway", ""));
    feature.set_field(StopsPlatformsStationIndexes::public_transport, object.get_value_by_key("public_transport", ""));
    feature.set_field(StopsPlatformsStationIndexes::highway, object.get_value_by_key("highway", ""));
    feature.set_field(StopsPlatformsStationIndexes::name, object.get_value_by_key("name", ""));
    feature.set_field(StopsPlatformsStationIndexes::network, object.get_value_by_key("network", ""));
    feature.set_field(StopsPlatformsStationIndexes::_operator, object.get_value_by_key("operator", ""));
    if (refs) {
        feature.set_field(StopsPlatformsIndexes::ref, object.get_value_by_key("ref", ""));
        feature.set_field(StopsPlatformsIndexes::local_ref, object.get_value_by_key("local_ref", ""));
    }
    if (amenity) {
        feature.set_field(StationsIndexes::amenity, object.get_value_by_key("amenity", ""));
    }
}

/*static*/ void RailwayHandlerPass1::set_node_id(gdalcpp::Feature& feature, const osmium::Node& node) {
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", node.id());
    feature.set_field(FieldIndexes::node_id, idbuffer);
}

/*static*/ void RailwayHandlerPass1::set_way_id(gdalcpp::Feature& feature, const osmium::Way& way) {
    static char idbuffer[20];
    sprintf(idbuffer, "%ld", way.id());
    feature.set_field(FieldIndexes::way_id, idbuffer);
}

void RailwayHandlerPass1::relation(const osmium::Relation&) {}
