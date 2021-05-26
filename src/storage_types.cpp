#include "storage_types.hpp"


void PackageQueue::push(Package&& package_ref) {
    queue_.emplace_back(std::move(package_ref));
}

Package PackageQueue::pop() {
    if (queue_type_ == PackageQueueType::FIFO) {
        Package temp_package = std::move(queue_.front());
        queue_.pop_front();
        return temp_package;
    } else {
        Package temp_package = std::move(queue_.back());
        queue_.pop_back();
        return temp_package;
    }
}