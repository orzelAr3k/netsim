#include "nodes.hpp"

void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
    double size = preferences.size();
    double probability = size / (size + 1);

    if (preferences.empty()) preferences.emplace(r, 1.);
    else {
        for (auto& elem: preferences) {
            elem.second *= probability;
        }
        preferences.emplace(r, 1 / (size + 1));
    }
}

IPackageReceiver* ReceiverPreferences::choose_receiver() {
    double probability = probability_gen_();
    double sum = 0;

    for (const auto& elem: preferences) {
        sum += elem.second;
        if (sum >= probability) {
            return elem.first;
        }
    }
    return nullptr;
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
    preferences.erase(r);
    double size = preferences.size();
    double probability = (size + 1) / size;

    for (auto& elem: preferences) {
        elem.second *= probability;
    }
}

void PackageSender::send_package() {
    if (sending_buffer) receiver_preferences_.choose_receiver()->receive_package(std::move(sending_buffer.value()));
    sending_buffer = std::nullopt;
}

void Ramp::deliver_goods(Time t) {
    if (di_ == 1) {
        Package package = Package();
        push_package(std::move(package));
    }
    if (t % di_ == 1) {
        Package package = Package();
        push_package(std::move(package));
    }

}

void Worker::do_work(Time t) {
    if (pd_ == 1) {
        if (!queue_->empty()) {
            push_package(queue_->pop());
            start_time_ = t;
        }
    } else {
        if (!worker_buffer_.has_value()) {
            if (!queue_->empty()) {
                worker_buffer_.emplace(queue_->pop());
                start_time_ = t;
            }
        }
        if (t - start_time_ == pd_ - 1) {
            push_package(std::move(worker_buffer_.value()));
            worker_buffer_ = std::nullopt;
        }
    }
}