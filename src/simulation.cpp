#include "simulation.hpp"
#include "types.hpp"


void simulate(Factory& factory, TimeOffset timeOffset, const std::function<void(Factory&, Time)>& rf) {
    if (factory.is_consistent()) {
        for (Time time = 1; time != timeOffset + 1; time++) {
            for (NodeCollection<Ramp>::iterator ramp = factory.ramp_begin(); ramp != factory.ramp_end(); ramp++) {
                ramp->deliver_goods(time);
                ramp->send_package();
            }
            for (NodeCollection<Worker>::iterator worker = factory.worker_begin();
                worker != factory.worker_end(); worker++) {
                worker->send_package();
                worker->do_work(time);
            }
            rf(factory, timeOffset);
        }
    } else throw std::logic_error("IS CONSISTANT ERROR!");
}