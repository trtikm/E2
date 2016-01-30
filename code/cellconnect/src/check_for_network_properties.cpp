#include <cellconnect/check_for_network_properties.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <ostream>

namespace cellconnect { namespace {



}}

namespace cellconnect {


void  add_degree_distributions(std::unordered_map<natural_32_bit,natural_64_bit> const&  addon,
                               std::unordered_map<natural_32_bit,natural_64_bit>& distribution_where_addon_will_be_added
                               )
{
    TMPROF_BLOCK();

    for (auto const& elem : addon)
    {
        auto const  it = distribution_where_addon_will_be_added.find(elem.first);
        if (it == distribution_where_addon_will_be_added.cend())
            distribution_where_addon_will_be_added.insert(elem);
        else
            it->second += elem.second;
    }
}

void  add_degree_distributions(std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
                               std::unordered_map<natural_32_bit,natural_64_bit>& distribution_where_all_addons_will_be_added
                               )
{
    TMPROF_BLOCK();

    for (auto const& addon : addons)
        add_degree_distributions(addon,distribution_where_all_addons_will_be_added);
}

void  add_degree_distributions(std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
                               std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&
                                        distribution_where_all_addons_will_be_added
                               )
{
    TMPROF_BLOCK();

    ASSUMPTION(distribution_where_all_addons_will_be_added.empty() ||
               addons.size() == distribution_where_all_addons_will_be_added.size());

    if (distribution_where_all_addons_will_be_added.empty())
        distribution_where_all_addons_will_be_added.resize(addons.size());
    for (natural_64_bit  i = 0ULL; i < addons.size(); ++i)
        add_degree_distributions(addons.at(i),distribution_where_all_addons_will_be_added.at(i));
}


}
