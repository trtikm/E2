#include <ai/skeletal_motion_templates.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/cortex.hpp>
#include <ai/cortex_mock.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <angeo/utility.hpp>
#include <utility/read_line.hpp>
#include <utility/hash_combine.hpp>
#include <utility/canonical_path.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <unordered_set>
#include <deque>

namespace ai { namespace detail { namespace {


void  open_file_stream_for_reading(std::ifstream&  istr, boost::filesystem::path const&  pathname)
{
    if (!boost::filesystem::is_regular_file(pathname))
        throw std::runtime_error(msgstream() << "Cannot access file '" << pathname << "'.");
    if (boost::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The file '" << pathname << "' is invalid (wrong size).");

    istr.open(pathname.string(), std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open file '" << pathname << "'.");
}


natural_32_bit  read_num_records(std::ifstream&  istr, boost::filesystem::path const&  pathname)
{
    natural_32_bit  num_records;
    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read number of records in the file '" << pathname << "'.");
        std::istringstream istr(line);
        istr >> num_records;
        if (num_records == 0U)
            throw std::runtime_error(msgstream() << "The the file '" << pathname << "' does not contain any records.");
    }
    return num_records;
}


template<typename elem_type, typename param_type>
void  read_meta_data_records(
        boost::filesystem::path const&  pathname,
        std::function<void(
                std::vector<elem_type>&,
                std::string const&,
                std::vector<param_type> const&,
                boost::filesystem::path const&,
                natural_32_bit,
                elem_type&)
                > const&  back_inserter,
        bool const  inform_about_keyframe_end,
        std::vector<elem_type>* const  output_flat,
        std::vector<std::vector<elem_type> >* const  output_composed
        )
{
    TMPROF_BLOCK();

    std::ifstream  istr;
    open_file_stream_for_reading(istr, pathname);

    natural_32_bit  line_index = 0U;

    natural_32_bit const  num_records = read_num_records(istr, pathname);
    ++line_index;

    if (output_flat != nullptr)
        output_flat->clear();
    if (output_composed != nullptr)
        output_composed->clear();

    std::string  keyword;
    std::vector<param_type>  params;

    elem_type  last;
    bool  starting_first_elem = true;

    while (true)
    {
        std::string  line = read_line(istr);
        if (line.empty())
            break;
        ++line_index;

        boost::algorithm::trim(line);
        if (line.empty() || (line.size() >= 2U && line.at(0) == '%' && line.at(1) == '%')) // Skip empty and comment lines.
            continue;

        if (line.front() == '+' || line.front() == '-' || std::isdigit(line.front(), std::locale::classic()))
        {
            if (keyword.empty())
                throw std::runtime_error(msgstream() << "Number is not expected at line " << line_index << "in the file '" << pathname << "'.");

            std::istringstream sstr(line);
            param_type  value;
            sstr >> value;
            params.push_back(value);
        }
        else
        {
            if (!keyword.empty())
            {
                if (output_flat != nullptr)
                    back_inserter(*output_flat, keyword, params, pathname, line_index, last);
                if (output_composed != nullptr)
                    back_inserter(output_composed->back(), keyword, params, pathname, line_index, last);

                keyword.clear();
                params.clear();
            }

            if (line.front() == '@')
            {
                if (inform_about_keyframe_end && !starting_first_elem)
                {
                    if (output_flat != nullptr)
                        back_inserter(*output_flat, "<<@>>", {}, pathname, line_index, last);
                    if (output_composed != nullptr)
                        back_inserter(output_composed->back(), "<<@>>", {}, pathname, line_index, last);
                }
                starting_first_elem = false;

                if (output_composed != nullptr)
                    output_composed->push_back({});
                line = line.substr(1U);
            }
            else if ((output_flat != nullptr && output_flat->empty()) || (output_composed != nullptr && output_composed->empty()))
                throw std::runtime_error(msgstream() << "A keyword without '@' prefix is not expected at line " << line_index << "in the file '" << pathname << "'.");

            keyword = line;
        }
    }

    if (!keyword.empty())
    {
        if (output_flat != nullptr)
            back_inserter(*output_flat, keyword, params, pathname, line_index, last);
        if (output_composed != nullptr)
            back_inserter(output_composed->back(), keyword, params, pathname, line_index, last);
    }

    if (inform_about_keyframe_end)
    {
        if (output_flat != nullptr)
            back_inserter(*output_flat, "<<@>>", {}, pathname, line_index, last);
        if (output_composed != nullptr)
            back_inserter(output_composed->back(), "<<@>>", {}, pathname, line_index, last);
    }

    if (output_flat != nullptr && num_records != output_flat->size())
        throw std::runtime_error(msgstream() << "The first line in the file '" << pathname << "' says there is "
                                             << num_records << " records, but there was " << output_flat->size()
                                             << " actually encountered in the file.");
    if (output_composed != nullptr && num_records != output_composed->size())
        throw std::runtime_error(msgstream() << "The first line in the file '" << pathname << "' says there is "
                                             << num_records << " records, but there was " << output_composed->size()
                                             << " actually encountered in the file.");
}


}}}

namespace ai { namespace detail { namespace meta {


mass_distributions_data::mass_distributions_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : data()
{
    TMPROF_BLOCK();

    read_meta_data_records<mass_distribution_ptr, float_32_bit>(
            finaliser->get_key().get_unique_id(),
            [](std::vector<mass_distribution_ptr>&  output,
               std::string const&  keyword,
               std::vector<float_32_bit> const&  params,
               boost::filesystem::path const&  pathname,
               natural_32_bit const  line_index,
               mass_distribution_ptr&  last
               ) -> void
            {
                if (params.size() != 10UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for mass_distribution at line " << line_index << "in the file '" << pathname << "'.");
                mass_distribution  md;
                md.mass_inverted = params.at(0);
                for (int i = 0; i != 3; ++i)
                    for (int j = 0; j != 3; ++j)
                        md.inertia_tensor_inverted(i, j) = params.at(1 + 3 * i + j);
                last = last != nullptr && *last == md ? last : std::make_shared<skeletal_motion_templates::mass_distribution>(md);
                output.push_back(last);
            },
            false,
            &data,
            nullptr
            );
}

mass_distributions_data::~mass_distributions_data()
{
    TMPROF_BLOCK();
}


}}}

namespace ai { namespace detail { namespace meta {


colliders_data::colliders_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : data()
{
    TMPROF_BLOCK();

    read_meta_data_records<collider_ptr, float_32_bit>(
            finaliser->get_key().get_unique_id(),
            [](std::vector<collider_ptr>&  output,
               std::string const&  keyword,
               std::vector<float_32_bit> const&  params,
               boost::filesystem::path const&  pathname,
               natural_32_bit const  line_index,
               collider_ptr&  last
               ) -> void
            {
                if (keyword == "capsule")
                {
                    if (params.size() != 3UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for capsule at line " << line_index << "in the file '" << pathname << "'.");
                    skeletal_motion_templates::collider_capsule  collider;
                    collider.length = params.at(0);
                    collider.radius = params.at(1);
                    collider.weight = params.at(2);
                    last = last != nullptr && *last == collider ? last : std::make_shared<skeletal_motion_templates::collider_capsule>(collider);
                    output.push_back(last);
                }
                else
                    NOT_IMPLEMENTED_YET();
            },
            false,
            &data,
            nullptr
            );
}

colliders_data::~colliders_data()
{
    TMPROF_BLOCK();
}


}}}

namespace ai { namespace detail { namespace meta {


std::string const  action_none::unique_name = "none";
std::string const  action_move_forward_with_ideal_speed::unique_name = "move_forward_with_ideal_speed";
std::string const  action_rotate_forward_vector_towards_desired_linear_velocity::unique_name = "rotate_forward_vector_towards_desired_linear_velocity";
std::string const  action_turn_around::unique_name = "turn_around";
std::string const  action_dont_move::unique_name = "dont_move";
std::string const  action_dont_rotate::unique_name = "dont_rotate";
std::string const  action_set_linear_velocity::unique_name = "set_linear_velocity";
std::string const  action_set_angular_velocity::unique_name = "set_angular_velocity";
std::string const  action_cancel_gravity_accel::unique_name = "cancel_gravity_accel";


template<typename data_type, typename base_data_type>
std::shared_ptr<base_data_type const>  _find_or_create_motion_action_component(
        data_type const&  key,
        std::vector<std::shared_ptr<base_data_type const> > const* const  cache
        )
{
    std::shared_ptr<base_data_type const>  data_ptr = nullptr;
    if (cache != nullptr)
        for (auto  ptr : *cache)
            if (*ptr == key)
            {
                data_ptr = ptr;
                break;
            }
    return data_ptr != nullptr ? data_ptr : std::make_shared<data_type const>(key);
}


motion_actions_data::motion_actions_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : data()
{
    TMPROF_BLOCK();

    using  mutable_guarded_actions_ptr = std::shared_ptr<guarded_actions>;

    bool last_was_action = false;
    guarded_actions  constructed_actions;
    read_meta_data_records<mutable_guarded_actions_ptr, float_32_bit>(
            finaliser->get_key().get_unique_id(),
            [&last_was_action, &constructed_actions](
                std::vector<mutable_guarded_actions_ptr>&  output,
                std::string const&  keyword,
                std::vector<float_32_bit> const&  params,
                boost::filesystem::path const&  pathname,
                natural_32_bit const  line_index,
                mutable_guarded_actions_ptr&  last
                ) -> void
            {
                ASSUMPTION(keyword.size() > 1UL);

                if (keyword == "<<@>>" || (last_was_action && !boost::starts_with(keyword, "[A]")))
                {
                    last = last != nullptr && *last == constructed_actions ? last : std::make_shared<guarded_actions>(constructed_actions);
                    if (last->predicates_positive.empty() && last->predicates_negative.empty())
                        throw std::runtime_error(msgstream() << "No positive nor negative guard in the motion action around line " << line_index << "in the file '" << pathname << "'.");
                    if (last->actions.empty())
                        throw std::runtime_error(msgstream() << "No action in the motion action around line " << line_index << "in the file '" << pathname << "'.");
                    output.push_back(last);
                    constructed_actions = guarded_actions();
                }

                if (boost::starts_with(keyword, "[GP]") || boost::starts_with(keyword, "[GN]"))
                {
                    last_was_action = false;

                    std::string const  guard_name = keyword.substr(4);

                    bool const  is_positive = boost::starts_with(keyword, "[GP]");
                    std::vector<constraint_ptr>&  predicates =
                        is_positive ? constructed_actions.predicates_positive : constructed_actions.predicates_negative ;
                    std::vector<constraint_ptr> const* const  last_predicates =
                        last == nullptr ? nullptr : is_positive ? &last->predicates_positive : &last->predicates_negative ;

                    if (guard_name == "contact_normal_cone")
                    {
                        if (params.size() != 4UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for contact_normal_cone at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::constraint_contact_normal_cone  constraint;
                        constraint.unit_axis(0) = params.at(0);
                        constraint.unit_axis(1) = params.at(1);
                        constraint.unit_axis(2) = params.at(2);
                        constraint.angle_in_radians = params.at(3);
                        predicates.push_back(_find_or_create_motion_action_component(constraint, last_predicates));
                    }
                    else if (guard_name == "has_any_contact")
                    {
                        if (!params.empty()) throw std::runtime_error(msgstream() << "Wrong number of parameters for has_any_contact at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::constraint_has_any_contact  constraint;
                        predicates.push_back(_find_or_create_motion_action_component(constraint, last_predicates));
                    }
                    else if (guard_name == "linear_velocity_in_falling_cone")
                    {
                        if (params.size() != 2UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for linear_velocity_in_falling_cone at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::constraint_linear_velocity_in_falling_cone  constraint;
                        constraint.cone_angle_in_radians = params.at(0);
                        constraint.min_linear_speed = params.at(1);
                        predicates.push_back(_find_or_create_motion_action_component(constraint, last_predicates));
                    }
                    else if (guard_name == "is_falling_linear_velocity")
                    {
                        if (params.size() != 1UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for is_falling_linear_velocity at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::constraint_is_falling_linear_velocity  constraint;
                        constraint.min_falling_speed = params.front();
                        predicates.push_back(_find_or_create_motion_action_component(constraint, last_predicates));
                    }
                    else if (guard_name == "desired_forward_vector_inside_cone")
                    {
                        if (params.size() != 4UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for desired_forward_vector_inside_cone at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::constraint_desired_forward_vector_inside_cone  constraint;
                        constraint.unit_axis(0) = params.at(0);
                        constraint.unit_axis(1) = params.at(1);
                        constraint.unit_axis(2) = params.at(2);
                        constraint.angle_in_radians = params.at(3);
                        predicates.push_back(_find_or_create_motion_action_component(constraint, last_predicates));
                    }
                    else if (guard_name == "always")
                    {
                        if (!params.empty()) throw std::runtime_error(msgstream() << "Wrong number of parameters for always at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::constraint_always  constraint;
                        predicates.push_back(_find_or_create_motion_action_component(constraint, last_predicates));
                    }
                    else
                        NOT_IMPLEMENTED_YET();
                }
                else if (boost::starts_with(keyword, "[A]"))
                {
                    last_was_action = true;

                    std::string const  action_name = keyword.substr(3);
                    std::vector<action_ptr> const* const  last_actions = last == nullptr ? nullptr : &last->actions;

                    if (action_name == "none")
                    {
                        if (!params.empty()) throw std::runtime_error(msgstream() << "Wrong number of parameters for meta action 'none' at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_none  action;
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "move_forward_with_ideal_speed")
                    {
                        if (params.size() != 2UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for move_forward_with_ideal_speed at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_move_forward_with_ideal_speed  action;
                        action.max_linear_accel = params.at(0);
                        action.motion_error_multiplier = params.at(1);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "rotate_forward_vector_towards_desired_linear_velocity")
                    {
                        if (params.size() != 3UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for rotate_forward_vector_towards_desired_linear_velocity at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_rotate_forward_vector_towards_desired_linear_velocity  action;
                        action.max_angular_speed = params.at(0);
                        action.max_angular_accel = params.at(1);
                        action.min_linear_speed = params.at(2);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "turn_around")
                    {
                        if (params.size() != 1UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for turn_around at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_turn_around  action;
                        action.max_angular_accel = params.at(0);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "dont_move")
                    {
                        if (params.size() != 2UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for dont_move at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_dont_move  action;
                        action.max_linear_accel = params.at(0);
                        action.radius = params.at(1);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "dont_rotate")
                    {
                        if (params.size() != 1UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for dont_rotate at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_dont_rotate  action;
                        action.max_angular_accel = params.at(0);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "set_linear_velocity")
                    {
                        if (params.size() != 4UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for set_linear_velocity at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_set_linear_velocity  action;
                        action.linear_velocity(0) = params.at(0);
                        action.linear_velocity(1) = params.at(1);
                        action.linear_velocity(2) = params.at(2);
                        action.max_linear_accel = params.at(3);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "set_angular_velocity")
                    {
                        if (params.size() != 4UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for set_angular_velocity at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_set_angular_velocity  action;
                        action.angular_velocity(0) = params.at(0);
                        action.angular_velocity(1) = params.at(1);
                        action.angular_velocity(2) = params.at(2);
                        action.max_angular_accel = params.at(3);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "cancel_gravity_accel")
                    {
                        if (!params.empty()) throw std::runtime_error(msgstream() << "Wrong number of parameters for meta action 'cancel_gravity_accel' at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_cancel_gravity_accel  action;
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else
                        NOT_IMPLEMENTED_YET();
                }
                else if (keyword == "<<@>>")
                {
                    last_was_action = false;
                    if (output.empty())
                        throw std::runtime_error(msgstream() << "Empty disjunction of guarded actions in the motion action around line " << line_index << "in the file '" << pathname << "'.");
                }
                else
                    UNREACHABLE();
            },
            true,
            nullptr,
            reinterpret_cast<std::vector<std::vector<mutable_guarded_actions_ptr> >*>(&data)
            );
}

motion_actions_data::~motion_actions_data()
{
    TMPROF_BLOCK();
}


}}}

namespace ai { namespace detail { namespace meta {


bool  free_bones_for_look_at::operator==(free_bones_for_look_at const&  other) const
{
    if (this == &other) return true;
    if (all_bones.size() != other.all_bones.size() || end_effector_bones.size() != other.end_effector_bones.size())
    {
        return false;
    }
    for (natural_32_bit i = 0; i != all_bones.size(); ++i)
    {
        if (all_bones.at(i) != other.all_bones.at(i))
        {
            return false;
        }
    }
    for (natural_32_bit i = 0; i != end_effector_bones.size(); ++i)
    {
        if (end_effector_bones.at(i) != other.end_effector_bones.at(i))
        {
            return false;
        }
    }
    return true;
}

free_bones_data::free_bones_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : look_at()
{
    TMPROF_BLOCK();

    read_meta_data_records<free_bones_for_look_at_ptr, natural_32_bit>(
            finaliser->get_key().get_unique_id(),
            [](std::vector<free_bones_for_look_at_ptr>&  output,
               std::string const&  keyword,
               std::vector<natural_32_bit> const&  params,
               boost::filesystem::path const&  pathname,
               natural_32_bit const  line_index,
               free_bones_for_look_at_ptr&  last
               ) -> void
            {
                if (params.size() % 2UL != 0UL)
                    throw std::runtime_error(msgstream() << "Wrong number of parameters around line " << line_index << "in the file '" << pathname << "'. Expected is even number of params.");
                if (keyword == "look_at")
                {
                    free_bones_for_look_at  record;
                    for (natural_32_bit i = 0U; i < params.size(); i += 2)
                    {
                        record.all_bones.push_back(params.at(i));
                        if (params.at(i + 1U) == 1U)
                            record.end_effector_bones.push_back(params.at(i));
                    }
                    last = last != nullptr && *last == record ? last : std::make_shared<free_bones_for_look_at>(record);
                    output.push_back(last);
                }
                else
                    throw std::runtime_error(msgstream() << "Unknown keyword " << keyword << " around line " << line_index << "in the file '" << pathname << "'. Expected is even number of params.");
            },
            false,
            &look_at,
            nullptr
            );
}

free_bones_data::~free_bones_data()
{
    TMPROF_BLOCK();
}


}}}

namespace ai { namespace detail {


bone_names_data::bone_names_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    std::ifstream  istr;
    open_file_stream_for_reading(istr, pathname);

    std::unordered_set<std::string>  visited;
    for (natural_32_bit i = 0U, num_names = read_num_records(istr, pathname); i != num_names; ++i)
    {
        std::string  line;
        if (!read_line(istr, line))
            throw std::runtime_error(msgstream() << "Cannot read name #" << i << " in the file '" << pathname << "'.");
        boost::algorithm::trim(line);

        if (visited.count(line) != 0UL)
            throw std::runtime_error(msgstream() << "Duplicate bone name '" << line << "' at line #" << i << " in the file '" << pathname << "'.");
        visited.insert(line);

        data.push_back(line);
    }
}

bone_names_data::~bone_names_data()
{
    TMPROF_BLOCK();
}


}}

namespace ai { namespace detail {


bone_hierarchy_data::bone_hierarchy_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    std::ifstream  istr;
    open_file_stream_for_reading(istr, pathname);

    std::unordered_set<std::string>  visited;
    for (natural_32_bit i = 0U, n = read_num_records(istr, pathname); i != n; ++i)
    {
        std::string  line;
        if (!read_line(istr, line))
            throw std::runtime_error(msgstream() << "Cannot read parent #" << i << " in the file '" << pathname << "'.");
        integer_32_bit  parent_index;
        {
            std::istringstream sstr(line);
            sstr >> parent_index;
        }
        if (parent_index < -1)
            throw std::runtime_error(msgstream() << "Wrong parent bone index " << parent_index << " for bone #" << i << " in the file '"
                                                 << pathname << "'. E.i. the bones were not saved in the topological order.");
        if (parent_index >(integer_32_bit)i)
            throw std::runtime_error(msgstream() << "Bone #" << i << " comes before its parent bone #" << parent_index << " in the file '"
                                                 << pathname << "'. E.i. the bones were not saved in the topological order.");
        parents.push_back(parent_index);
    }

    angeo::skeleton_compute_child_bones(parents, children);
}

bone_hierarchy_data::~bone_hierarchy_data()
{
    TMPROF_BLOCK();
}


}}

namespace ai { namespace detail {


bone_lengths_data::bone_lengths_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    std::ifstream  istr;
    open_file_stream_for_reading(istr, pathname);

    for (natural_32_bit i = 0U, n = read_num_records(istr, pathname); i != n; ++i)
    {
        std::string  line;
        if (!read_line(istr, line))
            throw std::runtime_error(msgstream() << "Cannot read length #" << i << " in the file '" << pathname << "'.");
        boost::algorithm::trim(line);

        float_32_bit  length;
        {
            std::istringstream sstr(line);
            sstr >> length;
        }
        if (length < 0.001f)
            throw std::runtime_error(msgstream() << "Wrong value " << length << " for length #" << i << " in the file '" << pathname << "'.");

        data.push_back(length);
    }
}

bone_lengths_data::~bone_lengths_data()
{
    TMPROF_BLOCK();
}


}}

namespace ai { namespace detail {


bone_joints_data::bone_joints_data(async::finalise_load_on_destroy_ptr const  finaliser)
	: data()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();
	std::unique_ptr<boost::property_tree::ptree> const  ptree(new boost::property_tree::ptree);
	if (!boost::filesystem::is_regular_file(pathname))
		throw std::runtime_error(msgstream() << "Cannot access the file '" << pathname << "'.");
	boost::property_tree::read_info(pathname.string(), *ptree);
	data.resize(ptree->size());
	natural_32_bit i = 0U;
	for (boost::property_tree::ptree::value_type const& bone_and_props : *ptree)
	{
		std::vector<angeo::joint_rotation_props>&  data_at_i = data.at(i);
		data_at_i.resize(bone_and_props.second.size());
		natural_32_bit j = 0U;
		for (boost::property_tree::ptree::value_type const& none_and_props : bone_and_props.second)
		{
			boost::property_tree::ptree const&  joint_ptree = none_and_props.second;
			angeo::joint_rotation_props&  joint = data_at_i.at(j);
			joint.m_axis = vector3(joint_ptree.get<float_32_bit>("axis.x"),
								   joint_ptree.get<float_32_bit>("axis.y"),
								   joint_ptree.get<float_32_bit>("axis.z"));
			joint.m_axis_in_parent_space = joint_ptree.get<bool>("axis_in_parent_space");
			joint.m_zero_angle_direction = vector3(joint_ptree.get<float_32_bit>("zero_angle_direction.x"),
												   joint_ptree.get<float_32_bit>("zero_angle_direction.y"),
												   joint_ptree.get<float_32_bit>("zero_angle_direction.z"));
			joint.m_direction = vector3(joint_ptree.get<float_32_bit>("direction.x"),
										joint_ptree.get<float_32_bit>("direction.y"),
										joint_ptree.get<float_32_bit>("direction.z"));
			joint.m_max_angle = joint_ptree.get<float_32_bit>("max_angle");
			joint.m_stiffness_with_parent_bone = joint_ptree.get<float_32_bit>("stiffness_with_parent_bone");
			joint.m_max_angular_speed = joint_ptree.get<float_32_bit>("max_angular_speed");

			++j;
		}
		++i;
	}
}

bone_joints_data::~bone_joints_data()
{
    TMPROF_BLOCK();
}


}}

namespace ai { namespace detail {


anim_space_directions_data::anim_space_directions_data(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    std::ifstream  istr;
    open_file_stream_for_reading(istr, pathname);

    char const* const  direction_names[2] = { "forward", "up" };
    vector3* const  directions[2] = { &forward, &up };
    for (natural_32_bit i = 0U; i != 2U; ++i)
    {
        for (natural_32_bit j = 0U; j != 3U; ++j)
        {
            std::string  line;
            if (!read_line(istr, line))
                throw std::runtime_error(msgstream() << "Cannot read coordinate #" << j
                                                     << " of " << direction_names[i] << " direction vector "
                                                     << " in the file '" << pathname << "'.");
            std::istringstream istr(line);
            istr >> (*directions[i])(j);
        }
        normalise(*directions[i]);
    }
}

anim_space_directions_data::~anim_space_directions_data()
{
    TMPROF_BLOCK();
}


}}

namespace ai { namespace detail {


motion_template_transitions_data::motion_template_transitions_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : data()
    , data_lookup()
    , mock()
    , initial_motion_name()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();
    if (!boost::filesystem::is_regular_file(pathname))
        throw std::runtime_error(msgstream() << "Cannot access the file '" << pathname << "'.");
    try
    {
        boost::property_tree::read_json(pathname.string(), data);
    }
    catch (boost::property_tree::json_parser_error const& e)
    {
        LOG(error, "Loading of '" << pathname.string() << "' has FAILED. Details: " << e.what());
        throw e;
    }

    initial_motion_name = data.begin()->first;
    for (auto const&  name_and_child : data)
        data_lookup.insert({ name_and_child.first, &name_and_child.second });

    boost::filesystem::path const  mock_pathname = pathname.parent_path() / "transitions.mock.json";
    if (!boost::filesystem::is_regular_file(mock_pathname))
        throw std::runtime_error(msgstream() << "Cannot access the file '" << mock_pathname << "'.");
    try
    {
        boost::property_tree::read_json(mock_pathname.string(), mock);
    }
    catch (boost::property_tree::json_parser_error const& e)
    {
        LOG(error, "Loading of '" << mock_pathname.string() << "' has FAILED. Details: " << e.what());
        throw e;
    }
}

motion_template_transitions_data::~motion_template_transitions_data()
{
    TMPROF_BLOCK();
}


void  motion_template_transitions_data::find_targets(
        motion_template_cursor const&  cursor,
        std::unordered_map<std::string, motion_template> const&  motions_map,
        std::vector<transition_info>&  output
        ) const
{
    TMPROF_BLOCK();

    motion_template const&  src_mt = motions_map.at(cursor.motion_name);

    auto const  get_successor_keyframe_index =
        [&motions_map, &cursor, &src_mt](property_tree const&  record, motion_template const&  dst_mt, natural_32_bit&  successor_index) -> bool {
            if (record.count("range") != 0UL)
            {
                property_tree const&  range = record.get_child("range");
                INVARIANT(range.size() == 2UL);
                int const  raw_lo = range.begin()->second.get_value<int>();
                int const  raw_hi = range.rbegin()->second.get_value<int>();
                natural_32_bit const  lo = (natural_32_bit)((raw_lo >= 0 ? 0 : (int)src_mt.keyframes.num_keyframes()) + raw_lo);
                natural_32_bit const  hi = (natural_32_bit)((raw_hi >= 0 ? 0 : (int)src_mt.keyframes.num_keyframes()) + raw_hi);
                if (cursor.keyframe_index < lo || hi < cursor.keyframe_index)
                    return false;
                if (record.count("to") != 0UL)
                {
                    property_tree const&  to = record.get_child("to");
                    INVARIANT(to.size() == 1UL);
                    int const  raw_index = to.begin()->second.get_value<int>();
                    successor_index = (natural_32_bit)((raw_index >= 0 ? 0 : (int)dst_mt.keyframes.num_keyframes()) + raw_index);
                }
                else
                {
                    INVARIANT(&src_mt == &dst_mt);
                    successor_index = cursor.keyframe_index + 1U;
                }
                return true;
            }
            else
            {
                property_tree const&  from = record.get_child("from");
                INVARIANT(!from.empty());
                if (record.count("to") != 0UL)
                {
                    property_tree const&  to = record.get_child("to");
                    INVARIANT(from.size() == to.size());
                    for (auto  from_it = from.begin(), to_it = to.begin(); from_it != from.end(); ++from_it, ++to_it)
                    {
                        int  raw_index = from_it->second.get_value<int>();
                        natural_32_bit const  src_index = (natural_32_bit)((raw_index >= 0 ? 0 : (int)src_mt.keyframes.num_keyframes()) + raw_index);
                        if (src_index == cursor.keyframe_index)
                        {
                            raw_index = to_it->second.get_value<int>();
                            successor_index = (natural_32_bit)((raw_index >= 0 ? 0 : (int)dst_mt.keyframes.num_keyframes()) + raw_index);
                            return true;
                        }
                    }
                }
                else
                {
                    INVARIANT(&src_mt == &dst_mt);
                    for (auto from_it = from.begin(); from_it != from.end(); ++from_it)
                    {
                        int const  raw_index = from_it->second.get_value<int>();
                        natural_32_bit const  src_index = (natural_32_bit)((raw_index >= 0 ? 0 : (int)src_mt.keyframes.num_keyframes()) + raw_index);
                        if (src_index == cursor.keyframe_index)
                        {
                            successor_index = cursor.keyframe_index + 1U;
                            return true;
                        }
                    }
                }
                return false;
            }
        };
    auto const  insert_record = [&output, this](
            property_tree const&  record,
            motion_template_cursor const&  successor_cursor,
            float_32_bit const  time
        ) -> void {
            property_tree const&  defaults = data_lookup.at(successor_cursor.motion_name)->get_child("defaults");
            property_tree const&  guard = (record.count("guard") != 0UL ? record : defaults).get_child("guard");
            property_tree const&  desire = (record.count("desire") != 0UL ? record : defaults).get_child("desire");
            output.push_back({ successor_cursor, time, &guard, &desire });
        };

    float_32_bit  next_frame_interpolation_time = 0.0f;
    {
        if (cursor.keyframe_index + 1UL < src_mt.keyframes.num_keyframes())
            next_frame_interpolation_time = src_mt.keyframes.time_point_at(cursor.keyframe_index + 1UL) -
                                            src_mt.keyframes.time_point_at(cursor.keyframe_index);
    }

    property_tree const&  ptree = *data_lookup.at(cursor.motion_name);

    for (auto const&  null_and_child : ptree.get_child("loops"))
    {
        natural_32_bit  successor_index;
        if (get_successor_keyframe_index(null_and_child.second, src_mt, successor_index))
            insert_record(
                    null_and_child.second,
                    { cursor.motion_name, successor_index },
                    null_and_child.second.get_child("time").get_value<float_32_bit>()
                    );
    }
    for (auto const&  null_and_child : ptree.get_child("successors"))
    {
        natural_32_bit  successor_index;
        if (get_successor_keyframe_index(null_and_child.second, src_mt, successor_index))
        {
            INVARIANT(next_frame_interpolation_time > 0.0f);
            insert_record(
                    null_and_child.second,
                    { cursor.motion_name, successor_index },
                    next_frame_interpolation_time
                    );
        }
    }
    for (auto const& name_and_records : ptree.get_child("exits"))
        for (auto const& null_and_child : name_and_records.second)
        {
            natural_32_bit  successor_index;
            if (get_successor_keyframe_index(null_and_child.second, motions_map.at(name_and_records.first), successor_index))
                insert_record(
                        null_and_child.second,
                        { name_and_records.first, successor_index },
                        null_and_child.second.get_child("time").get_value<float_32_bit>()
                        );
        }

    INVARIANT(!output.empty());
}


void  motion_template_transitions_data::__check_loaded_data__(std::unordered_map<std::string, motion_template> const&  motions_map) const
{
    TMPROF_BLOCK();

    std::string const  prefix = msgstream() << "motion_template_transitions_data::__check_loaded_data__: ";

    auto const  check_range_from_to = [&prefix](int range, int from, int to, std::string const&  name, std::string const&  kind, bool use_to) -> void {
        if (range == -2 && from == -1)
            throw std::runtime_error(msgstream() << prefix << "There is neither 'range' "
                                                    "nor 'from' key under '" << kind << "' section of source template motion name: " << name << ".");
        if (range != -2 && from != -1)
            throw std::runtime_error(msgstream() << prefix << "Both 'range' and 'from' "
                                                    "keys are present under '" << kind << "' section of source template motion name: " << name << ".");
        if (std::abs(range) != 2)
            throw std::runtime_error(msgstream() << prefix << "The 'range' array "
                                                    "under '" << kind << "' section of source template motion name '" << name << "' does not "
                                                    "contain exactly two integers.");
        if (std::abs(from) == 0)
            throw std::runtime_error(msgstream() << prefix << "The 'from' array "
                                                    "under '" << kind << "' section of source template motion name '" << name << "' does not "
                                                    "contain at least one integer.");
        if (!use_to)
            return;
        if (from > 0 && to != from)
            throw std::runtime_error(msgstream() << prefix << "The 'to' array "
                                                    "under '" << kind << "' section of source template motion name '" << name << "' does not "
                                                    "contain does not contain exactly the same number of integers as 'from' array.");
        if (range > 0 && to != 1)
            throw std::runtime_error(msgstream() << prefix << "The 'to' array "
                                                    "under '" << kind << "' section of source template motion name '" << name << "' does not "
                                                    "contain does not contain exactly one integer (required from the use of 'range' interval).");
    };

    for (auto const&  name_and_child : data)
    {
        if (motions_map.count(name_and_child.first) == 0UL)
            throw std::runtime_error(msgstream() << prefix << "Unknown source template motion name: "
                                                 << name_and_child.first << ".");
        int  num_loops = -1, num_successors = -1, num_exits = -1;
        for (auto const& kind_and_child : name_and_child.second)
        {
            if (kind_and_child.first == "defaults")
            {
                int i = 0;
                for (auto const& type_and_child : kind_and_child.second)
                {
                    ++i;
                    if (type_and_child.first == "guard")
                        cortex::__check_loaded_guard__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                          << name_and_child.first << "."
                                                                                          << kind_and_child.first << "."
                                                                                          << "[" << i << "]."
                                                                                          << type_and_child.first << "': ");
                    else if (type_and_child.first == "desire")
                        cortex::__check_loaded_desire__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                           << name_and_child.first << "."
                                                                                           << kind_and_child.first << "."
                                                                                           << "[" << i << "]."
                                                                                           << type_and_child.first << "': ");
                    else
                        throw std::runtime_error(msgstream() << prefix << "Unknown key '"
                                                             << type_and_child.first << "' under 'defaults' of source template motion name: "
                                                             << name_and_child.first << ".");
                }
            }
            else if (kind_and_child.first == "loops")
            {
                num_loops = (int)kind_and_child.second.size();
                int i = 0;
                for (auto const& null_and_child : kind_and_child.second)
                {
                    ++i;
                    int  range = -2, from = -1, to = -1, time = 0;
                    for (auto const& type_and_child : null_and_child.second)
                    {
                        if (type_and_child.first == "from")
                            from = (int)type_and_child.second.size();
                        else if (type_and_child.first == "range")
                            range = (int)type_and_child.second.size();
                        else if (type_and_child.first == "to")
                            to = (int)type_and_child.second.size();
                        else if (type_and_child.first == "time")
                            time = 1;
                        else if (type_and_child.first == "guard")
                            cortex::__check_loaded_guard__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                              << name_and_child.first << "."
                                                                                              << kind_and_child.first << "."
                                                                                              << "[" << i << "]."
                                                                                              << type_and_child.first << "': ");
                        else if (type_and_child.first == "desire")
                            cortex::__check_loaded_desire__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                               << name_and_child.first << "."
                                                                                               << kind_and_child.first << "."
                                                                                               << "[" << i << "]."
                                                                                               << type_and_child.first << "': ");
                    }
                    check_range_from_to(range, from, to, name_and_child.first, kind_and_child.first, true);
                    if (time == 0)
                        throw std::runtime_error(msgstream() << prefix << "Missing 'time' key "
                                                                "under 'loops' section of source template motion name: "
                                                             << name_and_child.first << ".");
                }
            }
            else if (kind_and_child.first == "successors")
            {
                num_successors = (int)kind_and_child.second.size();;
                int i = 0;
                for (auto const& null_and_child : kind_and_child.second)
                {
                    ++i;
                    int  range = -2, from = -1;
                    for (auto const& type_and_child : null_and_child.second)
                    {
                        if (type_and_child.first == "to" || type_and_child.first == "time")
                            throw std::runtime_error(msgstream() << prefix << "Illegal use of '"
                                                                 << type_and_child.first
                                                                 << "' key under 'successors' section of source template motion name: "
                                                                 << name_and_child.first << ".");
                        else if (type_and_child.first == "from")
                            from = (int)type_and_child.second.size();
                        else if (type_and_child.first == "range")
                            range = (int)type_and_child.second.size();
                        else if (type_and_child.first == "guard")
                            cortex::__check_loaded_guard__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                              << name_and_child.first << "."
                                                                                              << kind_and_child.first << "."
                                                                                              << "[" << i << "]."
                                                                                              << type_and_child.first << "': ");
                        else if (type_and_child.first == "desire")
                            cortex::__check_loaded_desire__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                               << name_and_child.first << "."
                                                                                               << kind_and_child.first << "."
                                                                                               << "[" << i << "]."
                                                                                               << type_and_child.first << "': ");
                    }
                    check_range_from_to(range, from, -1, name_and_child.first, kind_and_child.first, false);
                }
            }
            else if (kind_and_child.first == "exits")
            {
                num_exits = (int)kind_and_child.second.size();
                for (auto const& template_name_and_child : kind_and_child.second)
                {
                    if (motions_map.count(template_name_and_child.first) == 0UL)
                        throw std::runtime_error(msgstream() << prefix << "Unknown target "
                                                                "template motion name '"
                                                             << template_name_and_child.first
                                                             << "' under 'exits' section of source template motion name '"
                                                             << name_and_child.first << "'.");
                    if (template_name_and_child.second.empty())
                        throw std::runtime_error(msgstream() << prefix << "There is no record  "
                                                                "under target template motion '"
                                                             << template_name_and_child.first
                                                             << "' under 'exits' section of source template motion name '"
                                                             << name_and_child.first << "'.");
                    int i = 0;
                    for (auto const& null_and_child : template_name_and_child.second)
                    {
                        ++i;
                        int  range = -2, from = -1, to = -1, time = 0;
                        for (auto const& type_and_child : null_and_child.second)
                        {
                            if (type_and_child.first == "from")
                                from = (int)type_and_child.second.size();
                            else if (type_and_child.first == "range")
                                range = (int)type_and_child.second.size();
                            else if (type_and_child.first == "to")
                                to = (int)type_and_child.second.size();
                            else if (type_and_child.first == "time")
                                time = 1;
                            else if (type_and_child.first == "guard")
                                cortex::__check_loaded_guard__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                                  << name_and_child.first << "."
                                                                                                  << kind_and_child.first << "."
                                                                                                  << template_name_and_child.first << "."
                                                                                                  << "[" << i << "]."
                                                                                                  << type_and_child.first << "': ");
                            else if (type_and_child.first == "desire")
                                cortex::__check_loaded_desire__(type_and_child.second, msgstream() << prefix << "In '"
                                                                                                   << name_and_child.first << "."
                                                                                                   << kind_and_child.first << "."
                                                                                                   << template_name_and_child.first << "."
                                                                                                   << "[" << i << "]."
                                                                                                   << type_and_child.first << "': ");
                        }
                        check_range_from_to(range, from, to, name_and_child.first, kind_and_child.first, true);
                        if (time == 0)
                            throw std::runtime_error(msgstream() << prefix << "Missing 'time' key "
                                                                    "under 'exits' section of target template motion name '"
                                                                 << template_name_and_child.first << "' of source template motion name: "
                                                                 << name_and_child.first << ".");
                    }
                }
            }
            else
                throw std::runtime_error(msgstream() << prefix << "Unknown key '"
                                                     << kind_and_child.first << "' under source template motion name: "
                                                     << name_and_child.first << ".");
        }
        if (num_loops == -1)
            throw std::runtime_error(msgstream() << prefix << "Missing key 'loops' "
                                                    "under source template motion name: " << name_and_child.first << ".");
        if (num_successors == -1)
            throw std::runtime_error(msgstream() << prefix << "Missing key 'successors' "
                                                    "under source template motion name: " << name_and_child.first << ".");
        if (num_exits == -1)
            throw std::runtime_error(msgstream() << prefix << "Missing key 'exits' "
                                                    "under source template motion name: " << name_and_child.first << ".");
        if (num_loops + num_exits == 0)
            throw std::runtime_error(msgstream() << prefix << "The source template motion name: " << name_and_child.first
                                                 << "is a dead end.");
    }

    cortex_mock::__check_loaded_data__(mock, prefix);
}


}}

namespace ai { namespace detail {


skeletal_motion_templates_data::skeletal_motion_templates_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : pose_frames()
    , names()
    , hierarchy()
    , lengths()
    , directions()
    , motions_map()
{
    TMPROF_BLOCK();

    async::finalise_load_on_destroy_ptr const  ultimate_finaliser = async::finalise_load_on_destroy::create(
            [this](async::finalise_load_on_destroy_ptr) -> void {

                // All data are loaded. So, let's check for their consistency.

                if (pose_frames.empty())
                    throw std::runtime_error("The 'pose_frames' were not loaded.");
                if (names.empty())
                    throw std::runtime_error("The 'names' were not loaded.");
                if (hierarchy.empty())
                    throw std::runtime_error("The 'hierarchy' was not loaded.");
                if (lengths.empty())
                    throw std::runtime_error("The 'lengths' were not loaded.");
                if (directions.empty())
                    throw std::runtime_error("The 'directions' were not loaded.");
                if (motions_map.empty())
                    throw std::runtime_error("The 'motions_map' is empty.");
                if (joints.empty())
                    throw std::runtime_error("The 'joints' is empty.");

                if (pose_frames.size() == 0U)
                    throw std::runtime_error("The 'pose_frames' is empty.");
                if (pose_frames.size() != names.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'names' are different.");
                if (pose_frames.size() != hierarchy.parents().size())
                    throw std::runtime_error("The size of 'pose_frames' and 'hierarchy.parents()' are different.");
                if (pose_frames.size() != lengths.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'lengths' are different.");
                if (joints.size() != names.size())
                    throw std::runtime_error("The size of 'joints' and 'names' are different.");

                if (hierarchy.parent(0U) != -1)
                    throw std::runtime_error("Invariant failure: hierarchy.parent(0) != -1.");

                for (auto const&  entry : motions_map)
                {
                    motion_template const&  record = entry.second;

                    if (record.keyframes.empty())
                        throw std::runtime_error(msgstream() << "The 'keyframes' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.reference_frames.empty())
                        throw std::runtime_error(msgstream() << "The 'reference_frames' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.mass_distributions.empty())
                        throw std::runtime_error(msgstream() << "The 'mass_distributions' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.colliders.empty())
                        throw std::runtime_error(msgstream() << "The 'colliders' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.actions.empty())
                        throw std::runtime_error(msgstream() << "The 'motion_actions' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.free_bones.empty())
                        throw std::runtime_error(msgstream() << "The 'free_bones' were not loaded to 'motions_map[" << entry.first << "]'.");

                    if (record.keyframes.num_keyframes() < 2U)
                        throw std::runtime_error(msgstream() << "The count of keyframes is less than 2 in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.reference_frames.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'reference_frames' differ in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.mass_distributions.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'mass_distributions' differ in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.colliders.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'colliders' differ in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.actions.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'motion_actions' differ in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.free_bones.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'free_bones' differ in 'motions_map[" << entry.first << "]'.");
                }

                transitions.__check_loaded_data__(motions_map);
            },
            finaliser
            );

    std::unordered_set<std::string>  visited;
    std::deque<boost::filesystem::path>  work_list{ finaliser->get_key().get_unique_id() };
    do
    {
        boost::filesystem::path const  pathname = work_list.front();
        work_list.pop_front();

        if (visited.count(pathname.string()) != 0UL)
            continue;
        visited.insert(pathname.string());

        if (!boost::filesystem::is_directory(pathname))
            throw std::runtime_error(msgstream() << "Cannot access directory '" << pathname << "'.");

        if (boost::filesystem::is_regular_file(pathname / "imports.txt"))
        {
            std::ifstream  istr;
            open_file_stream_for_reading(istr, pathname / "imports.txt");
            for (std::string  line = read_line(istr); !line.empty(); line = read_line(istr))
            {
                boost::algorithm::trim(line);
                if (!line.empty())
                    work_list.push_back(canonical_path(pathname / line));
            }
        }

        if (pose_frames.empty() && boost::filesystem::is_regular_file(pathname / "pose.txt"))
            pose_frames = modelspace(pathname / "pose.txt", 10U, ultimate_finaliser);
        if (names.empty() && boost::filesystem::is_regular_file(pathname / "names.txt"))
            names = bone_names(pathname / "names.txt", 10U, ultimate_finaliser);
        if (hierarchy.empty() && boost::filesystem::is_regular_file(pathname / "parents.txt"))
            hierarchy = bone_hierarchy(pathname / "parents.txt", 10U, ultimate_finaliser);
        if (lengths.empty() && boost::filesystem::is_regular_file(pathname / "lengths.txt"))
            lengths = bone_lengths(pathname / "lengths.txt", 10U, ultimate_finaliser);
        if (directions.empty() && boost::filesystem::is_regular_file(pathname / "directions.txt"))
            directions = anim_space_directions(pathname / "directions.txt", 10U, ultimate_finaliser);
        if (joints.empty() && boost::filesystem::is_regular_file(pathname / "joints.txt"))
            joints = bone_joints(pathname / "joints.txt", 10U, ultimate_finaliser);
        if (transitions.empty() && boost::filesystem::is_regular_file(pathname / "transitions.json"))
            transitions = motion_template_transitions(pathname / "transitions.json", 10U, ultimate_finaliser);

        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(pathname))
            if (boost::filesystem::is_directory(entry.path()))
            {
                boost::filesystem::path const  animation_pathname = canonical_path(entry.path());
                std::string const  animation_name = animation_pathname.filename().string();
                motion_template&  record = motions_map[animation_name];
                bool  has_keyframes = false;
                for (boost::filesystem::directory_entry& animation_entry : boost::filesystem::directory_iterator(animation_pathname))
                    if (boost::filesystem::is_regular_file(animation_entry.path()))
                    {
                        if (has_keyframes == false)
                        {
                            std::string const  filename = animation_entry.path().filename().string();
                            std::string const  extension = animation_entry.path().filename().extension().string();
                            if (filename.find("keyframe") == 0UL && extension == ".txt")
                                has_keyframes = true;
                        }

                        if (record.reference_frames.empty() && boost::filesystem::is_regular_file(animation_pathname / "meta_reference_frames.txt"))
                            record.reference_frames = meta::reference_frames(animation_pathname / "meta_reference_frames.txt", 1U, ultimate_finaliser, "ai::skeletal_motion_templates_data::reference_frames");
                        if (record.mass_distributions.empty() && boost::filesystem::is_regular_file(animation_pathname / "meta_mass_distributions.txt"))
                            record.mass_distributions = meta::mass_distributions(animation_pathname / "meta_mass_distributions.txt", 1U, ultimate_finaliser);
                        if (record.colliders.empty() && boost::filesystem::is_regular_file(animation_pathname / "meta_motion_colliders.txt"))
                            record.colliders = meta::colliders(animation_pathname / "meta_motion_colliders.txt", 1U, ultimate_finaliser);
                        if (record.actions.empty() && boost::filesystem::is_regular_file(animation_pathname / "meta_motion_actions.txt"))
                            record.actions = meta::motion_actions(animation_pathname / "meta_motion_actions.txt", 1U, ultimate_finaliser);
                        if (record.free_bones.empty() && boost::filesystem::is_regular_file(animation_pathname / "meta_free_bones.txt"))
                            record.free_bones = meta::free_bones(animation_pathname / "meta_free_bones.txt", 1U, ultimate_finaliser);
                    }
                if (has_keyframes)
                    record.keyframes = keyframes(animation_pathname, 1U, ultimate_finaliser);
            }
    }
    while (!work_list.empty());
}


skeletal_motion_templates_data::~skeletal_motion_templates_data()
{
    TMPROF_BLOCK();
}


}}

namespace ai {


std::size_t skeletal_motion_templates::motion_template_cursor::hasher::operator()(
        skeletal_motion_templates::motion_template_cursor const&  value
        ) const
{
    std::size_t seed = 0;
    ::hash_combine(seed, value.motion_name);
    ::hash_combine(seed, value.keyframe_index);
    return seed;
}


}
