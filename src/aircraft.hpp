#pragma once

#include "GL/displayable.hpp"
#include "aircraft_types.hpp"
#include "config.hpp"
#include "geometry.hpp"
#include "tower.hpp"
#include "waypoint.hpp"

#include <string>
#include <string_view>

class Aircraft : public GL::Displayable, public GL::DynamicObject
{
private:
    const AircraftType& type;
    const std::string flight_number;
    Point3D pos, speed; // note: the speed should always be normalized to length 'speed'
    WaypointQueue waypoints = {};
    Tower& control;
    bool landing_gear_deployed = false; // is the landing gear deployed?
    bool is_at_terminal        = false;
    bool fpass                 = true;
    int fuel                   = 150 + std::rand() % 2850;

    bool is_circling() const;
    bool has_terminal() const;

    void turn_to_waypoint();
    void turn(Point3D direction);

    unsigned int get_speed_octant() const;
    void arrive_at_terminal();
    void operate_landing_gear();
    template <const bool front> void add_waypoint(const Waypoint& wp);
    bool is_on_ground() const { return pos.z() < DISTANCE_THRESHOLD; }
    float max_speed() const { return is_on_ground() ? type.max_ground_speed : type.max_air_speed; }

    Aircraft(const Aircraft&) = delete;
    Aircraft& operator=(const Aircraft&) = delete;

public:
    Aircraft(const AircraftType& type_, const std::string_view& flight_number_, const Point3D& pos_,
             const Point3D& speed_, Tower& control_) :
        GL::Displayable { pos_.x() + pos_.y() },
        type { type_ },
        flight_number { flight_number_ },
        pos { pos_ },
        speed { speed_ },
        control { control_ }
    {
        speed.cap_length(max_speed());
    }

    const std::string& get_flight_num() const { return flight_number; }
    float distance_to(const Point3D& p) const { return pos.distance_to(p); }

    void display() const override;
    bool move() override;
    int fuel_level() const { return fuel; }
    bool is_low_on_fuel() const { return fuel < 200; }
    bool has_served() const { return !fpass; };

    void refill(int& fuel_stock)
    {
        assert(fuel_stock);
        auto needed_fuel = std::min(3000 - fuel, fuel_stock);
        fuel += needed_fuel;
        fuel_stock -= needed_fuel;
        std::cout << flight_number << " got refilled using " << needed_fuel << "L of fuel" << std::endl;
        assert(fuel_stock >= 0);
        assert(fuel > 0);
    }

    friend class Tower;
};
