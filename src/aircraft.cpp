#include "aircraft.hpp"

#include "GL/opengl_interface.hpp"

#include <cmath>

void Aircraft::turn_to_waypoint()
{
    if (!waypoints.empty())
    {
        Point3D target = waypoints[0];
        if (waypoints.size() > 1)
        {
            const float d   = (waypoints[0] - pos).length();
            const Point3D W = (waypoints[0] - waypoints[1]).normalize(d / 2.0f);
            target += W;
        }

        turn(target - pos - speed);
    }
}

void Aircraft::turn(Point3D direction)
{
    (speed += direction.cap_length(type.max_accel)).cap_length(max_speed());
}

unsigned int Aircraft::get_speed_octant() const
{
    const float speed_len = speed.length();
    if (speed_len > 0)
    {
        const Point3D norm_speed { speed * (1.0f / speed_len) };
        const float angle =
            (norm_speed.y() > 0) ? 2.0f * 3.141592f - std::acos(norm_speed.x()) : std::acos(norm_speed.x());
        
        return (static_cast<int>(std::round((angle * NUM_AIRCRAFT_TILES) / (2.0f * 3.141592f))) + 1) %
               NUM_AIRCRAFT_TILES;
    }
    else
    {
        return 0;
    }
}

void Aircraft::arrive_at_terminal()
{
    control.arrived_at_terminal(*this);
    is_at_terminal = true;
}

void Aircraft::operate_landing_gear()
{
    if (waypoints.size() > 1u)
    {
        const auto it            = waypoints.begin();
        const bool ground_before = it->is_on_ground();
        const bool ground_after  = std::next(it)->is_on_ground();
        if (ground_before && !ground_after)
        {
            std::cout << flight_number << " lift off" << std::endl;
        }
        else if (!ground_before && ground_after)
        {
            std::cout << flight_number << " is now landing..." << std::endl;
            landing_gear_deployed = true;
        }
        else if (!ground_before && !ground_after)
        {
            landing_gear_deployed = false;
        }
    }
}

template <const bool front> void Aircraft::add_waypoint(const Waypoint& wp)
{
    if (front)
    {
        waypoints.push_front(wp);
    }
    else
    {
        waypoints.push_back(wp);
    }
}

bool Aircraft::move()
{
    if (waypoints.empty())
    {
        if (!fpass)
        {
            return true;
        }
        for (const auto& wp: control.get_instructions(*this))
        {
            add_waypoint<false>(wp);
        }
    }
    if (is_circling() && waypoints.empty())
    {
        WaypointQueue new_path = control.reserve_terminal(*this);
        if (!new_path.empty())
        {
            waypoints = std::move(new_path);
        }
    }

    if (!is_at_terminal)
    {
        turn_to_waypoint();
        pos += speed;
        if (!waypoints.empty() && distance_to(waypoints.front()) < DISTANCE_THRESHOLD)
        {
            if (waypoints.front().is_at_terminal())
            {
                arrive_at_terminal();
            }
            else
            {
                operate_landing_gear();
            }
            waypoints.pop_front();
        }

        if (is_on_ground())
        {
            if (!landing_gear_deployed)
            {
                using namespace std::string_literals;
                throw AircraftCrash { flight_number + " crashed into the ground"s };
            }
        }
        else
        {
            if (--fuel == 0)
            {
                throw AircraftCrash { flight_number + " ran out of fuel" };
            }
            const float speed_len = speed.length();
            if (speed_len < SPEED_THRESHOLD)
            {
                pos.z() -= SINK_FACTOR * (SPEED_THRESHOLD - speed_len);
            }
        }

        // update the z-value of the displayable structure
        GL::Displayable::z = pos.x() + pos.y();
    }
    return false;
}

bool Aircraft::has_terminal() const
{

    return !waypoints.empty() && waypoints.back().is_at_terminal();
}

bool Aircraft::is_circling() const
{
    return !fpass && !waypoints.empty() && !has_terminal();
}

void Aircraft::display() const
{
    type.texture.draw(project_2D(pos), { PLANE_TEXTURE_DIM, PLANE_TEXTURE_DIM }, get_speed_octant());
}

