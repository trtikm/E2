#include <ai/skeletal_motion_templates.hpp>
#include <ai/skeleton_utils.hpp>
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
    , initial_motion_name()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    std::ifstream  istr;
    open_file_stream_for_reading(istr, pathname);

    natural_32_bit  line_index = 0U;
    natural_32_bit  column = 0U;

    while (true)
    {
        std::string  line = read_line(istr, &line_index);
        if (line == "digraph G {")
            break;
    }
    while (true)
    {
        std::string  line = read_line(istr, &line_index);
        if (!line.empty() && line.front() == '}')
            break;
        if (is_empty_line(line))
            continue;
        try
        {
            column = 0U;

            std::string  src_anim_name;
            {
                while (line.at(column) != '"') ++column; ++column;
                natural_32_bit const  begin = column;
                while (line.at(column) != '"') ++column;
                if (column - begin == 0U)
                    throw std::runtime_error(msgstream() << "Empty animation name.");
                src_anim_name = line.substr(begin, column - begin);
                ++column;
            }
            std::string  dst_anim_name;
            {
                while (line.at(column) != '"') ++column; ++column;
                natural_32_bit const  begin = column;
                while (line.at(column) != '"') ++column;
                if (column - begin == 0U)
                    throw std::runtime_error(msgstream() << "Empty animation name.");
                dst_anim_name = line.substr(begin, column - begin);
                ++column;
            }
            float_32_bit  time_in_seconds;
            {
                while (line.at(column) != '"') ++column; ++column;
                natural_32_bit const  begin = column;
                while (line.at(column) != '\\') ++column;
                if (column - begin == 0U)
                    throw std::runtime_error(msgstream() << "No transition time specified.");
                time_in_seconds = (float_32_bit)std::atof(line.substr(begin, column - begin).c_str());
                if (time_in_seconds <= 0.0f)
                    throw std::runtime_error(msgstream() << "The transition time is not greater than 0.0f.");
                ++column;
                ++column;
            }
            natural_32_bit  src_keyframe_index;
            {
                natural_32_bit const  begin = column;
                while (line.at(column) != ';') ++column;
                src_keyframe_index = (column - begin == 0U) ? std::numeric_limits<natural_32_bit>::max() :  // this will be resolved later (once we know count of keyframes)
                    std::atoi(line.substr(begin, column - begin).c_str());
                ++column;
            }
            natural_32_bit  dst_keyframe_index;
            {
                natural_32_bit const  begin = column;
                while (line.at(column) != '"') ++column;
                dst_keyframe_index = (column - begin == 0U) ? 0U : std::atoi(line.substr(begin, column - begin).c_str());
            }

            motion_template_cursor const  src_cursor{ src_anim_name, src_keyframe_index };
            motion_template_cursor const  dst_cursor{ dst_anim_name, dst_keyframe_index };

            auto  targets_range = data.equal_range(src_cursor);
            for (auto it = targets_range.first; it != targets_range.second; ++it)
                if (it->second.first == dst_cursor)
                    throw std::runtime_error("Duplicate transition (note: transition time is NOT considered).");

            data.insert({ src_cursor, { dst_cursor, time_in_seconds} });
            if (initial_motion_name.empty())
                initial_motion_name = src_cursor.motion_name;
        }
        catch (std::exception const&  e)
        {
            throw std::runtime_error(msgstream() << "[line=" << line_index << ", column=" << column << "]: '" << line << "': "<< e.what());
        }
    }
}

motion_template_transitions_data::~motion_template_transitions_data()
{
    TMPROF_BLOCK();
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

                // Here we resolve 'std::numeric_limits<natural_32_bit>::max()' keyframe indices in the transitions graph.
                {
                    motion_template_transitions_data::transitions_graph  resolved_graph;
                    for (auto const& src_and_tgt_and_time : transitions.data())
                        resolved_graph.insert({
                            {
                                src_and_tgt_and_time.first.motion_name,
                                src_and_tgt_and_time.first.keyframe_index == std::numeric_limits<natural_32_bit>::max() ?
                                        (natural_32_bit)(motions_map.at(src_and_tgt_and_time.first.motion_name).keyframes.num_keyframes() - 1U) :
                                        src_and_tgt_and_time.first.keyframe_index
                                },
                            src_and_tgt_and_time.second
                            });
                    const_cast<motion_template_transitions_data::transitions_graph&>(transitions.data()).swap(resolved_graph);
                }

                // And we rather check for consistency of the transitions graph.
                for (auto const&  src_and_tgt_and_time : transitions.data())
                {
                    std::vector<std::string> const  names{
                        src_and_tgt_and_time.first.motion_name,
                        src_and_tgt_and_time.second.first.motion_name
                    };
                    std::vector<natural_32_bit> const  indices{
                        src_and_tgt_and_time.first.keyframe_index,
                        src_and_tgt_and_time.second.first.keyframe_index
                    };
                    std::vector<std::string> const  type{ "source", "target" };
                    for (int i = 0; i != 2; ++i)
                    {
                        if (motions_map.count(names.at(i)) == 0UL)
                            throw std::runtime_error(msgstream() << "The 'transitions' graph contains unknown " << type.at(i) << " animation name '" << names.at(i) << ".");
                        if (motions_map.at(names.at(i)).keyframes.num_keyframes() <= indices.at(i))
                            throw std::runtime_error(msgstream() << "The 'transitions' graph contains wrong keyframe index '" << indices.at(i)<< "' for the " << type.at(i) << " animation name '" << names.at(i) << ".");
                    }
                }
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
        if (transitions.empty() && boost::filesystem::is_regular_file(pathname / "transitions.dot"))
            transitions = motion_template_transitions(pathname / "transitions.dot", 10U, ultimate_finaliser);

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


natural_32_bit  skeletal_motion_templates_data::get_outdegree_of_keyframe(motion_template_cursor const& cursor) const
{
    natural_32_bit  count = cursor.keyframe_index + 1U < motions_map.at(cursor.motion_name).keyframes.get_keyframes().size() ? 1U : 0U;
    auto const  range = transitions.find_targets(cursor);
    count += (natural_32_bit)std::distance(range.first, range.second);
    return  count;
}


bool  skeletal_motion_templates_data::is_branching_keyframe(motion_template_cursor const& cursor) const
{
    return get_outdegree_of_keyframe(cursor) > 1U;
}


void  skeletal_motion_templates_data::get_successor_keyframes(
        motion_template_cursor const& cursor,
        std::vector<std::pair<motion_template_cursor /* target_cursor */, float_32_bit /* transition_time */> >&  output
        ) const
{
    for_each_successor_keyframe(
        cursor,
        [this, &output](skeletal_motion_templates::motion_template_cursor const&  c, float_32_bit const  t)
        {
            output.push_back({ c, t });
            return true;
        });
}


void  skeletal_motion_templates_data::for_each_successor_keyframe(
        motion_template_cursor const&  cursor,
        std::function<bool(motion_template_cursor const& /* target_cursor */, float_32_bit /* transition_time */)> const&  callback
        ) const

{
    auto const&  motion_template = motions_map.at(cursor.motion_name);
    if (cursor.keyframe_index + 1U < motion_template.keyframes.get_keyframes().size())
        if (!callback(
                { cursor.motion_name, cursor.keyframe_index + 1U },
                motion_template.keyframes.keyframe_at(cursor.keyframe_index + 1U).get_time_point()
                    - motion_template.keyframes.keyframe_at(cursor.keyframe_index).get_time_point()))
            return;
    auto const  range = transitions.find_targets(cursor);
    for (auto it = range.first; it != range.second; ++it)
        if (!callback(it->second.first, it->second.second))
            return;
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
