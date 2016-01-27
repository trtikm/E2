#include <cellconnect/check_for_network_properties.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>

namespace cellconnect { namespace {



}}

namespace cellconnect {


void  add_degree_distributions(std::unordered_map<natural_32_bit,natural_64_bit> const&  addon,
                               std::unordered_map<natural_32_bit,natural_64_bit>& ditribution_where_addon_will_be_added
                               )
{
    for (auto const& elem : addon)
    {
        auto const  it = ditribution_where_addon_will_be_added.find(elem.first);
        if (it == ditribution_where_addon_will_be_added.cend())
            ditribution_where_addon_will_be_added.insert(elem);
        else
            it->second += elem.second;
    }
}

void  add_degree_distributions(std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
                               std::unordered_map<natural_32_bit,natural_64_bit>& ditribution_where_all_addons_will_be_added
                               )
{
    for (auto const& addon : addons)
        add_degree_distributions(addon,ditribution_where_all_addons_will_be_added);
}


}
