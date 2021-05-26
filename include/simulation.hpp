#ifndef NETSIM_SIMULATION_HPP
#define NETSIM_SIMULATION_HPP

#include "factory.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

void simulate(Factory& factory, TimeOffset timeOffset, const std::function<void(Factory&, Time)>& rf);


#endif //NETSIM_SIMULATION_HPP
