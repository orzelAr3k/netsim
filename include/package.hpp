#ifndef NETSIM_PACKAGE_HPP
#define NETSIM_PACKAGE_HPP

#include <cmath>
#include <set>

#include "types.hpp"

class Package {
public:
    Package();
    Package(ElementID);
    Package(Package&&) noexcept;
    Package& operator=(Package&&) noexcept;

    ~Package();
    ElementID get_id() const { return ID; }
    bool relevance = false;

private:
    ElementID ID;
    static std::set<ElementID> assigned_IDs;
    static std::set<ElementID> freed_IDs;

protected:
    void make_irrelevant() { relevance = false; }
    void make_relevant() { relevance = true; }

};
#endif //NETSIM_PACKAGE_HPP
