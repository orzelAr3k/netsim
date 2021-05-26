#ifndef NETSIM_STORAGE_TYPES_HPP
#define NETSIM_STORAGE_TYPES_HPP

#include "package.hpp"

#include <iostream>
#include <list>


enum class PackageQueueType {
    LIFO, FIFO
};

class IPackageStockpile {
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual void push(Package&& package_ref) = 0;

    virtual bool empty() const = 0;

    virtual size_t size() const = 0;

    //iterators
    virtual const_iterator begin() const = 0;

    virtual const_iterator cbegin() const = 0;

    virtual const_iterator end() const = 0;

    virtual const_iterator cend() const = 0;

    virtual ~IPackageStockpile() = default;

};

class IPackageQueue : public IPackageStockpile {
public:
    virtual Package pop() = 0;

    virtual PackageQueueType get_queue_type() const = 0;

};

class PackageQueue : public IPackageQueue {
public:
    PackageQueue(PackageQueueType queue_type) : queue_type_(queue_type) {}

    void push(Package&& package_ref) override;

    bool empty() const override { return queue_.empty(); };

    size_t size() const override { return queue_.size(); };

    Package pop() override;

    PackageQueueType get_queue_type() const override { return queue_type_; };

    //iterators
    const_iterator begin() const override { return queue_.begin(); }

    const_iterator cbegin() const override { return queue_.cbegin(); }

    const_iterator end() const override { return queue_.end(); }

    const_iterator cend() const override { return queue_.cend(); }

protected:
    std::list<Package> queue_;
    PackageQueueType queue_type_;
};

#endif //NETSIM_STORAGE_TYPES_HPP