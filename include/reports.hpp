#ifndef NETSIM_REPORTS_HPP
#define NETSIM_REPORTS_HPP

#include "factory.hpp"
#include <tuple>
#include <sstream>
#include <fstream>
#include <functional>
#include <utility>




void generate_structure_report(const Factory&, std::ostream&);

void generate_simulation_turn_report(const Factory&, std::ostream&, Time);

std::map<std::string, std::pair<TimeOffset, std::set<std::string>>> map_ramps(const Factory&);

std::map<std::string, std::tuple<TimeOffset, PackageQueueType, std::set<std::string>>> map_workers(const Factory&);

std::set<std::string> map_storehouses(const Factory&);

class SpecificTurnsReportNotifier {
    std::set<Time> turns;
public:
    SpecificTurnsReportNotifier(std::set<Time> set) : turns(set) {};

    bool should_generate_report(Time time) { return (turns.find(time) != turns.end()); }
};

class IntervalReportNotifier {
    TimeOffset interval = 1;
public:
    IntervalReportNotifier(TimeOffset timeOffset) : interval(timeOffset) {};

    bool should_generate_report(Time time) { return ((time - 1) % interval == 0); }
};


#endif //NETSIM_REPORTS_HPP