#ifndef NETSIM_FACTORY_HPP
#define NETSIM_FACTORY_HPP

#ifndef NETSIM_IMPLEMENTATION_FACTORY_HPP
#define NETSIM_IMPLEMENTATION_FACTORY_HPP

#include <list>
#include <memory>
#include <algorithm>
#include "types.hpp"
#include "nodes.hpp"

enum class NodeColor {
    UNVISITED,
    VISITED,
    VERIFIED
};

enum class ElementType {
    LOADING_RAMP,
    WORKER,
    STOREHOUSE,
    LINK
};

template<typename Node>
class NodeCollection {
public:
    using container_t = typename std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;


    void add(Node&& node) { collect_.emplace_back(std::move(node)); };

    void remove_by_id(ElementID id) { if (collect_.end() != find_by_id(id)) collect_.erase(find_by_id(id)); }

    iterator find_by_id(ElementID id) {
        return std::find_if(collect_.begin(), collect_.end(),
                            [id](auto& it) { return it.get_id() == id; });
    }

    const_iterator find_by_id(ElementID id) const {
        return std::find_if(collect_.cbegin(), collect_.cend(), [id](const auto& it) {
            return it.get_id() == id;
        });
    }

    iterator begin() { return collect_.begin(); };

    iterator end() { return collect_.end(); };

    const_iterator begin() const { return collect_.begin(); };

    const_iterator cbegin() const { return collect_.cbegin(); };

    const_iterator end() const { return collect_.end(); };

    const_iterator cend() const { return collect_.cend(); };

private:
    container_t collect_;
};


class Factory {
public:
    Factory() = default;

    void do_package_passing();

    void do_work(Time t);

    bool is_consistent() const;

    void do_deliveries(Time t);


    void add_ramp(Ramp&& ramp) { ramps_.add(std::move(ramp)); }

    void remove_ramp(ElementID id) { ramps_.remove_by_id(id); }

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) { return ramps_.find_by_id(id); }

    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const { return ramps_.find_by_id(id); }

    NodeCollection<Ramp>::iterator ramp_begin() { return ramps_.begin(); };

    NodeCollection<Ramp>::iterator ramp_end() { return ramps_.end(); };

    NodeCollection<Ramp>::const_iterator ramp_cbegin() const { return ramps_.cbegin(); };

    NodeCollection<Ramp>::const_iterator ramp_cend() const { return ramps_.cend(); };


    void add_worker(Worker&& worker) { workers_.add(std::move(worker)); }

    void remove_worker(ElementID id) { remove_receiver(workers_, id); }

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) { return workers_.find_by_id(id); }

    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const { return workers_.find_by_id(id); }

    NodeCollection<Worker>::iterator worker_begin() { return workers_.begin(); }

    NodeCollection<Worker>::iterator worker_end() { return workers_.end(); }

    NodeCollection<Worker>::const_iterator worker_cbegin() const { return workers_.cbegin(); }

    NodeCollection<Worker>::const_iterator worker_cend() const { return workers_.cend(); }


    void add_storehouse(Storehouse&& storehouse) { storehouses_.add(std::move(storehouse)); }

    void remove_storehouse(ElementID id) { remove_receiver(storehouses_, id); }

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) { return storehouses_.find_by_id(id); }

    NodeCollection<Storehouse>::const_iterator
    find_storehouse_by_id(ElementID id) const { return storehouses_.find_by_id(id); }

    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const { return storehouses_.cbegin(); }

    NodeCollection<Storehouse>::const_iterator storehouse_cend() const { return storehouses_.cend(); }

private:
    template<typename Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id);

    NodeCollection<Ramp> ramps_;
    NodeCollection<Worker> workers_;
    NodeCollection<Storehouse> storehouses_;
};

template<typename Node>
void Factory::remove_receiver(NodeCollection<Node>& collection, ElementID id) {
    IPackageReceiver* iter = &(*collection.find_by_id(id));

    for (auto& workers: workers_) {
        for (auto& receiver : workers.receiver_preferences_.get_preferences()) {
            if (receiver.first == iter) {
                workers.receiver_preferences_.remove_receiver(receiver.first);
                break;
            }
        }
    }
    for (auto& ramps: ramps_) {
        for (auto& receiver : ramps.receiver_preferences_.get_preferences()) {
            if (receiver.first == iter) {
                ramps.receiver_preferences_.remove_receiver(receiver.first);
                break;
            }
        }
    }

    collection.remove_by_id(id);
}


struct ParsedLineData {
    ElementType element_type;
    std::map<std::string, std::string> parameters;
};


Factory load_factory_structure(std::istream& is);

void save_factory_structure(Factory& factory, std::ostream& os);

void tokenize(std::string& str, std::vector<std::string>& ct, char delimiter);

ParsedLineData parse(const std::string& l);

#endif //NETSIM_IMPLEMENTATION_FACTORY_HPP
#endif //NETSIM_FACTORY_HPP
