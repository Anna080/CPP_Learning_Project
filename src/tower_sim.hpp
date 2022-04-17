#pragma once

#include "aircraft_manager.hpp"
#include "aircraft_factory.hpp"

class Airport;
struct AircraftType;

struct ContextInitializer
{
    ContextInitializer(int argc, char** argv)
    {
        MediaPath::initialize(argv[0]);
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        GL::init_gl(argc, argv, "Airport Tower Simulation");
    };
};

class TowerSimulation
{
private:
    bool help        = false;
    Airport* airport = nullptr;

    TowerSimulation(const TowerSimulation&) = delete;
    TowerSimulation& operator=(const TowerSimulation&) = delete;
    ContextInitializer contextInitializer;
    AircraftManager manager {};
    AircraftFactory factory {};

    void create_random_aircraft() const;

    void create_keystrokes() const;
    void display_help() const;

    void init_airport();

public:
    TowerSimulation(int argc, char** argv);
    ~TowerSimulation();

    void launch();
};
