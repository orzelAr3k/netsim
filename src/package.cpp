#include "package.hpp"


std::set<ElementID> Package::assigned_IDs;
std::set<ElementID> Package::freed_IDs;


Package::Package() {
    if (freed_IDs.empty() and assigned_IDs.empty()) {
        ID = 1;
        assigned_IDs.emplace(ID);
    } else if (freed_IDs.empty() and !assigned_IDs.empty()) {
        ID = *assigned_IDs.rbegin() + 1;
        assigned_IDs.emplace(ID);
    } else {
        ID = *freed_IDs.begin();
        freed_IDs.erase(ID);
        assigned_IDs.emplace(ID);
    }
    make_relevant();
}

Package::Package(ElementID elementId) {
    if (assigned_IDs.find(elementId) != assigned_IDs.end()) {
        Package();
    } else {
        ID = elementId;
        freed_IDs.erase(ID);
        assigned_IDs.emplace(ID);
    }
    make_relevant();
}

Package::~Package() {
    if (relevance) {
        ElementID ID_ = this->ID;
        assigned_IDs.erase(ID_);
        freed_IDs.insert(ID_);
    }
}

Package::Package(Package&& aPackage) noexcept {
    ID = aPackage.ID;
    aPackage.make_irrelevant();
    this->make_relevant();
}

Package& Package::operator=(Package&& aPackage) noexcept {
    if (relevance) {
        assigned_IDs.erase(ID);
        if (ID < *assigned_IDs.rend())
            freed_IDs.insert(ID);
    }
    ID = aPackage.ID;
    aPackage.make_irrelevant();
    this->make_relevant();
    return *this;
}