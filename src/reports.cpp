#include "reports.hpp"

void generate_structure_report(const Factory& factory, std::ostream& ostream) {
    auto ramps = map_ramps(factory);
    auto workers = map_workers(factory);
    auto shs = map_storehouses(factory);
    ostream << "\n== LOADING RAMPS ==\n\n";
    for (const auto& it:ramps) {
        ostream << it.first << "\n" << "  Delivery interval: " << it.second.first << "\n" << "  Receivers:\n";
        for (const auto& rec:it.second.second)
            ostream << "    " << rec << "\n";
        ostream << "\n";
    }
    ostream << "\n== WORKERS ==\n\n";
    for (const auto& it:workers) {
        ostream << it.first << "\n" << "  Processing time: " << std::get<0>(it.second) << "\n" << "  Queue type: " <<
                ((std::get<1>(it.second) == PackageQueueType::FIFO) ? "FIFO" : "LIFO") << "\n" << "  Receivers:\n";
        for (const auto& rec : std::get<2>(it.second))
            ostream << "    " << rec << "\n";
        ostream << "\n";
    }
    ostream << "\n== STOREHOUSES ==\n\n";
    for (const auto& it:shs)
        ostream << it << "\n\n";
}

void generate_simulation_turn_report(const Factory& factory, std::ostream& ostream, Time time) {
    ostream << "=== [ Turn: " << time << " ] ===\n";
    std::map<ElementID, const Worker*> workers;
    for (auto it = factory.worker_cbegin(); it != factory.worker_cend(); it++) {
        workers[it->get_id()] = &*it;
    }
    std::map<ElementID, const Storehouse*> shs;
    for (auto it = factory.storehouse_cbegin(); it != factory.storehouse_cend(); it++) {
        shs[it->get_id()] = &*it;
    }
    ostream << "\n== WORKERS ==\n\n";
    for (auto it:workers) {
        ostream << "WORKER #" << it.first << "\n";
        auto& proc = it.second->get_processing_buffer();
        ostream << "  PBuffer: ";
        if (proc.has_value())
            ostream << "#" << proc->get_id() << " (pt = " << it.second->get_package_processing_start_time() << ")\n";
        else ostream << "(empty)\n";
        ostream << "  Queue:";
        if (it.second->empty()) ostream << " (empty)";
        size_t commas = it.second->size();
        if (commas != 0) commas--;
        for (auto pack = it.second->begin(); pack != it.second->end(); pack++, commas--)
            ostream << " #" << pack->get_id() << (commas ? "," : "");
        ostream << "\n";
        auto& send = it.second->get_sending_buffer();
        ostream << "  SBuffer: ";
        if (send.has_value())
            ostream << "#" << send->get_id() << "\n";
        else ostream << "(empty)\n";
        ostream << "\n";
    }
    ostream << "\n== STOREHOUSES ==\n\n";
    for (auto it:shs) {
        ostream << "STOREHOUSE #" << it.first << "\n" << "  Stock:";
        if (it.second->empty()) ostream << " (empty)";
        size_t commas = it.second->size();
        if (commas != 0) commas--;
        for (auto pack = it.second->begin(); pack != it.second->end(); pack++, commas--)
            ostream << " #" << pack->get_id() << (commas ? "," : "");
        ostream << "\n\n";
    }
}

std::map<std::string, std::pair<TimeOffset, std::set<std::string>>> map_ramps(const Factory& factory) {
    std::map<std::string, std::pair<TimeOffset, std::set<std::string>>> ret;
    for (auto it = factory.ramp_cbegin(); it != factory.ramp_cend(); it++) {
        std::stringstream ramp;
        ramp << "LOADING RAMP #" << it->get_id();
        std::set<std::string> receivers;
        for (auto rec = it->receiver_preferences_.cbegin(); rec != it->receiver_preferences_.cend(); rec++) {
            std::stringstream ss;
            ss << "worker #" << rec->first->get_id();
            receivers.insert(ss.str());
        }
        ret[ramp.str()] = std::make_pair<TimeOffset, std::set<std::string>>(it->get_delivery_interval(),
                                                                            std::move(receivers));
    }
    return ret;
}

std::map<std::string, std::tuple<TimeOffset, PackageQueueType, std::set<std::string>>>
map_workers(const Factory& factory) {
    std::map<std::string, std::tuple<TimeOffset, PackageQueueType, std::set<std::string>>> ret;
    for (auto it = factory.worker_cbegin(); it != factory.worker_cend(); it++) {
        std::stringstream worker;
        worker << "WORKER #" << it->get_id();
        std::set<std::string> receivers;
        for (auto rec = it->receiver_preferences_.cbegin(); rec != it->receiver_preferences_.cend(); rec++) {
            std::stringstream ss;
            ss << ((rec->first->get_receiver_type() == ReceiverType::WORKER) ? "worker #" : "storehouse #")
               << rec->first->get_id();
            receivers.insert(ss.str());
        }
        ret[worker.str()] = std::make_tuple(it->get_processing_duration(), it->get_queue_type(), std::move(receivers));
    }
    return ret;
}

std::set<std::string> map_storehouses(const Factory& factory) {
    std::set<std::string> ret;
    for (auto it = factory.storehouse_cbegin(); it != factory.storehouse_cend(); it++) {
        std::stringstream sh;
        sh << "STOREHOUSE #" << it->get_id();
        ret.insert(sh.str());
    }
    return ret;
}