#ifndef NETSIM_NODES_HPP
#define NETSIM_NODES_HPP

#include <map>
#include <memory>
#include <functional>
#include <utility>
#include <optional>
#include "storage_types.hpp"
#include "package.hpp"
#include "helpers.hpp"
#include <config.hpp>

enum class ReceiverType {
    WORKER, STOREHOUSE
};

class IPackageReceiver {
public:
    virtual ElementID get_id() const = 0;

    virtual ReceiverType get_receiver_type() const = 0;

    virtual void receive_package(Package&& p) = 0;

    //iterators
    virtual IPackageStockpile::const_iterator begin() const = 0;

    virtual IPackageStockpile::const_iterator cbegin() const = 0;

    virtual IPackageStockpile::const_iterator end() const = 0;

    virtual IPackageStockpile::const_iterator cend() const = 0;

    virtual ~IPackageReceiver() = default;
};

class ReceiverPreferences {
public:
    using preferences_t = std::map<IPackageReceiver*, double>;
    using const_iterator = preferences_t::const_iterator;

    explicit ReceiverPreferences(ProbabilityGenerator probability_gen = probability_generator) : probability_gen_(
            std::move(probability_gen)) {};

    void add_receiver(IPackageReceiver* r);

    void remove_receiver(IPackageReceiver* r);

    const preferences_t& get_preferences() const { return preferences; }

    IPackageReceiver* choose_receiver();

    //iterators
    const_iterator begin() { return preferences.begin(); }

    const_iterator cbegin() const { return preferences.cbegin(); }

    const_iterator end() { return preferences.end(); }

    const_iterator cend() const { return preferences.cend(); }

    preferences_t preferences;

protected:
    ProbabilityGenerator probability_gen_;
};

class PackageSender {
public:
    using opt = std::optional<Package>;


    PackageSender() = default;

    PackageSender(PackageSender&&) = default;

    void send_package();

    const opt& get_sending_buffer() const { return sending_buffer; }

    ReceiverPreferences receiver_preferences_;

protected:
    opt sending_buffer;

    void push_package(Package&& aPackage) { sending_buffer = std::move(aPackage); }

};

class Ramp : public PackageSender {
public:
    Ramp(ElementID id, TimeOffset di) : id_(id), di_(di) {};

    void deliver_goods(Time t);

    TimeOffset get_delivery_interval() const { return di_; }

    ElementID get_id() const { return id_; }

private:
    ElementID id_;
    TimeOffset di_;
};

class Worker : public IPackageReceiver, public PackageSender, public IPackageQueue {
public:
    Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q) : id_(id), pd_(pd), start_time_(0),
                                                                            queue_(std::move(q)) {};

    void do_work(Time t);

    TimeOffset get_processing_duration() const { return pd_; };

    Time get_package_processing_start_time() const { return start_time_; };

    IPackageQueue* get_queue() const { return queue_.get(); };

    const std::optional<Package>& get_processing_buffer() const { return worker_buffer_; };

    void receive_package(Package&& p) override { queue_->push(std::move(p)); };

    ElementID get_id() const override { return id_; };

    ReceiverType get_receiver_type() const override { return receiverType_; };


    void push(Package&& p) override { queue_->push(std::move(p)); };

    bool empty() const override { return queue_->empty(); };

    std::size_t size() const override { return queue_->size(); };

    PackageQueueType get_queue_type() const override { return queue_->get_queue_type(); };

    Package pop() override { return queue_->pop(); }

    const_iterator begin() const override { return queue_->begin(); }

    const_iterator cbegin() const override { return queue_->cbegin(); }

    const_iterator end() const override { return queue_->end(); }

    const_iterator cend() const override { return queue_->cend(); }

private:
    ElementID id_;
    TimeOffset pd_;
    Time start_time_;
    std::unique_ptr<IPackageQueue> queue_;
    std::optional<Package> worker_buffer_;
    ReceiverType receiverType_ = ReceiverType::WORKER;
};

class Storehouse : public IPackageReceiver, public IPackageStockpile {
public:
    explicit Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d = std::make_unique<PackageQueue>(
            PackageQueue(PackageQueueType::FIFO))) : id_(id), d_(std::move(d)) {};

    void receive_package(Package&& aPackage) override { d_->push(std::move(aPackage)); }

    ElementID get_id() const override { return id_; };

    ReceiverType get_receiver_type() const override { return receiverType_; };

    void push(Package&& p) override { d_->push(std::move(p)); };

    bool empty() const override { return d_->empty(); };

    std::size_t size() const override { return d_->size(); };

    const_iterator begin() const override { return d_->begin(); }

    const_iterator cbegin() const override { return d_->cbegin(); }

    const_iterator end() const override { return d_->end(); }

    const_iterator cend() const override { return d_->cend(); }

private:
    ElementID id_;
    ReceiverType receiverType_ = ReceiverType::STOREHOUSE;
    std::unique_ptr<IPackageStockpile> d_;
};

#endif //NETSIM_NODES_HPP
