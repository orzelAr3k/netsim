#include "factory.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <typeinfo>
#include <string>
#include <sstream>


bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& node_color_map) {
    if (node_color_map[sender] == NodeColor::VERIFIED) {
        return true;
    }
    node_color_map[sender] = NodeColor::VISITED;

    if (sender->receiver_preferences_.preferences.empty()) {
        throw std::logic_error("The sender has no recipients!");
    }
    bool reach_store = false;
    for (const auto& receiver : sender->receiver_preferences_.preferences) {
        if (receiver.first->get_receiver_type() == ReceiverType::STOREHOUSE) {
            reach_store = true;
        } else {
            IPackageReceiver* receiver_ptr = receiver.first;
            auto worker_ptr = dynamic_cast<Worker*>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender*>(worker_ptr);

            if (sendrecv_ptr == sender) {
                continue;
            }

            if (node_color_map[sendrecv_ptr] == NodeColor::UNVISITED) {
                reach_store = has_reachable_storehouse(sendrecv_ptr, node_color_map);
            }
        }
    }
    node_color_map[sender] = NodeColor::VERIFIED;

    if (reach_store) {
        return true;
    } else {
        throw std::logic_error("ACCESS DENIED!");
    }

}

bool Factory::is_consistent() const {
    std::map<const PackageSender*, NodeColor> map_col;

    for (const auto& ramp : ramps_) {
        map_col[&ramp] = NodeColor::UNVISITED;
    }
    for (const auto& worker : workers_) {
        map_col[&worker] = NodeColor::UNVISITED;
    }

    try {
        for (const auto& ramp : ramps_) {
            has_reachable_storehouse(&ramp, map_col);
        }
    } catch (std::logic_error&) {
        return false;
    }
    return true;
}


void Factory::do_work(Time t) {
    for (auto& worker: workers_) {
        worker.do_work(t);
    }
}

void Factory::do_deliveries(Time t) {
    for (auto& ramp: ramps_) {
        ramp.deliver_goods(t);
    }
}

void Factory::do_package_passing() {
    for (auto& worker: workers_) {
        worker.send_package();
    }
}


void tokenize(std::string& str, std::vector<std::string>& ct, char delimiter) {
    std::istringstream stream(str);
    std::string tok;
    while (getline(stream, tok, delimiter)) {
        ct.push_back(tok);
    }
}


void save_factory_structure(Factory& factory, std::ostream& os) {
    std::vector<int> id_ramps;
    for (auto it = factory.ramp_cbegin(); it != factory.ramp_cend(); it++) {
        id_ramps.push_back(it->get_id());
    }
    std::sort(id_ramps.begin(), id_ramps.end());

    os << "; == LOADING RAMPS ==" << std::endl << std::endl;
    for (const auto ID : id_ramps) {
        auto iter = factory.find_ramp_by_id(ID);
        os << "LOADING_RAMP id=" << iter->get_id() << " delivery-interval=" << iter->get_delivery_interval()
           << std::endl;
        os << std::endl;
    }

    std::vector<int> worker_vec;
    for (auto it = factory.worker_cbegin(); it != factory.worker_cend(); it++) {
        worker_vec.push_back(it->get_id());
    }
    std::sort(worker_vec.begin(), worker_vec.end());

    os << "; == WORKERS ==" << std::endl << std::endl;
    for (const auto ID : worker_vec) {
        auto iter = factory.find_worker_by_id(ID);
        os << "WORKER id=" << iter->get_id() << " processing-time=" << iter->get_processing_duration()
           << " queue-type=";

        switch (iter->get_queue_type()) {
            case PackageQueueType::FIFO:
                os << "FIFO" << std::endl;
                break;
            case PackageQueueType::LIFO:
                os << "LIFO" << std::endl;
                break;
        }
    }

    std::vector<int> id_store;
    for (auto it = factory.storehouse_cbegin(); it != factory.storehouse_cend(); it++) {
        id_store.push_back(it->get_id());
    }
    std::sort(id_store.begin(), id_store.end());

    os << "; == STOREHOUSES ==" << std::endl << std::endl;
    for (auto ID : id_store) os << "STOREHOUSE id=" << ID << std::endl;
    os << std::endl;

    //Links:
    os << "; == LINKS ==" << std::endl << std::endl;
    //ramps as src:
    for (const auto ID : id_ramps) {
        auto iter = factory.find_ramp_by_id(ID);
        std::vector<int> worker_id;
        std::vector<int> store_id;
        for (const auto receiver: iter->receiver_preferences_.preferences) {
            switch (receiver.first->get_receiver_type()) {
                case ReceiverType::WORKER:
                    worker_id.push_back(receiver.first->get_id());
                    continue;
                case ReceiverType::STOREHOUSE:
                    store_id.push_back(receiver.first->get_id());
                    continue;
            }
        }
        std::sort(worker_id.begin(), worker_id.end());
        std::sort(store_id.begin(), store_id.end());

        for (auto id: store_id) os << "LINK src=ramp-" << ID << " dest=store-" << id << std::endl;
        for (auto id: worker_id) os << "LINK src=ramp-" << ID << " dest=worker-" << id << std::endl;
        os << std::endl;
    }

    for (const auto ID : worker_vec) {
        auto iter = factory.find_worker_by_id(ID);
        std::vector<int> worker_id;
        std::vector<int> store_id;
        for (const auto receiver: iter->receiver_preferences_.preferences) {
            switch (receiver.first->get_receiver_type()) {
                case ReceiverType::WORKER:
                    worker_id.push_back(receiver.first->get_id());
                    break;
                case ReceiverType::STOREHOUSE:
                    store_id.push_back(receiver.first->get_id());
                    break;
            }
        }
        std::sort(worker_id.begin(), worker_id.end());
        std::sort(store_id.begin(), store_id.end());

        for (auto id: store_id) os << "LINK src=worker-" << ID << " dest=store-" << id << std::endl;
        for (auto id: worker_id) os << "LINK src=worker-" << ID << " dest=worker-" << id << std::endl;
        os << std::endl;
    }
}


Factory load_factory_structure(std::istream& is) {
    ParsedLineData parsed_line;
    Factory factory;
    std::string l;
    std::string id = "id";

    while (std::getline(is, l)) {
        if (l.empty() or (l.front() == ';')) {
            continue;
        }
        parsed_line = parse(l);
        if (parsed_line.element_type == ElementType::LOADING_RAMP) {
            Ramp ramp(std::stoi(parsed_line.parameters[id]), std::stoi(parsed_line.parameters["delivery-interval"]));
            factory.add_ramp(std::move(ramp));
        }
        if (parsed_line.element_type == ElementType::WORKER) {
            PackageQueueType type;
            if (parsed_line.parameters["queue-type"] == "FIFO") type = PackageQueueType::FIFO;
            if (parsed_line.parameters["queue-type"] == "LIFO") type = PackageQueueType::LIFO;
            Worker worker(std::stoi(parsed_line.parameters[id]), std::stoi(parsed_line.parameters["processing-time"]),
                          std::make_unique<PackageQueue>(type));
            factory.add_worker(std::move(worker));
        }
        if (parsed_line.element_type == ElementType::STOREHOUSE) {
            Storehouse storehouse(std::stoi(parsed_line.parameters[id]));
            factory.add_storehouse(std::move(storehouse));
        }
        if (parsed_line.element_type == ElementType::LINK) {

       
            std::vector<std::string> src;
            std::vector<std::string> dest;
            char delimeter = '-';

            tokenize(parsed_line.parameters["src"], src, delimeter);
            tokenize(parsed_line.parameters["dest"], dest, delimeter);

            if (src[0] == "ramp") {
                if (dest[0] == "worker") {
                    auto const ramp_iter = factory.find_ramp_by_id(std::stoi(src[1]));
                    auto const worker_iter = factory.find_worker_by_id(std::stoi(dest[1]));
                    ramp_iter->receiver_preferences_.add_receiver(&*worker_iter);
                }
                if (dest[0] == "store") {
                    auto const ramp_iter = factory.find_ramp_by_id(std::stoi(src[1]));
                    auto const store_iter = factory.find_storehouse_by_id(std::stoi(dest[1]));
                    ramp_iter->receiver_preferences_.add_receiver(&*store_iter);
                }
            }
            if (src[0] == "worker") {
                if (dest[0] == "worker") {
                    auto const worker_src_iter = factory.find_worker_by_id(std::stoi(src[1]));
                    auto const worker_dst_iter = factory.find_worker_by_id(std::stoi(dest[1]));
                    worker_src_iter->receiver_preferences_.add_receiver(&*worker_dst_iter);
                }
                if (dest[0] == "store") {
                    auto const worker_src_iter = factory.find_worker_by_id(std::stoi(src[1]));
                    auto const store_iter = factory.find_storehouse_by_id(std::stoi(dest[1]));
                    worker_src_iter->receiver_preferences_.add_receiver(&*store_iter);
                }
            }
        }
    }
    return factory;
}


ParsedLineData parse(const std::string& l) {
    ParsedLineData parsed_line;

    int marker = 0;
    std::vector<std::string> token_vec;
    char delimiter = ' ';
    std::stringstream ss(l);
    std::string tok;

    while (std::getline(ss, tok, delimiter)) {
        if (marker == 0) {
            if (tok == "LOADING_RAMP") {
                parsed_line.element_type = ElementType::LOADING_RAMP;
                marker += 1;
                continue;
            }
            if (tok == "WORKER") {
                parsed_line.element_type = ElementType::WORKER;
                marker += 1;
                continue;
            }
            if (tok == "STOREHOUSE") {
                parsed_line.element_type = ElementType::STOREHOUSE;
                marker += 1;
                continue;
            }
            if (tok == "LINK") {
                parsed_line.element_type = ElementType::LINK;
                marker += 1;
                continue;
            }
        }
        token_vec.push_back(tok);
    }


    for (auto& elem : token_vec) {
        delimiter = '=';
        std::vector<std::string> pair;
        tokenize(elem, pair, delimiter);
        parsed_line.parameters[pair[0]] = pair[1];
    }
    return parsed_line;
}
