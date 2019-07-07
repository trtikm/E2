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
                    else if (action_name == "chase_ideal_linear_velocity")
                    {
                        if (params.size() != 2UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for chase_ideal_linear_velocity at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_chase_ideal_linear_velocity  action;
                        action.max_linear_accel = params.at(0);
                        action.motion_error_multiplier = params.at(1);
                        constructed_actions.actions.push_back(_find_or_create_motion_action_component(action, last_actions));
                    }
                    else if (action_name == "chase_linear_velocity_by_forward_vector")
                    {
                        if (params.size() != 3UL) throw std::runtime_error(msgstream() << "Wrong number of parameters for chase_linear_velocity_by_forward_vector at line " << line_index << "in the file '" << pathname << "'.");
                        skeletal_motion_templates::action_chase_linear_velocity_by_forward_vector  action;
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


keyframe_equivalences_data::keyframe_equivalences_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : data()
{
    TMPROF_BLOCK();

    read_meta_data_records<motion_template_cursor, natural_32_bit>(
            finaliser->get_key().get_unique_id(),
            [](std::vector<motion_template_cursor>&  output,
               std::string const&  keyword,
               std::vector<natural_32_bit> const&  params,
               boost::filesystem::path const&  pathname,
               natural_32_bit const  line_index,
               motion_template_cursor&
               ) -> void
            {
                if (params.empty())
                    throw std::runtime_error(msgstream() << "Wrong number of parameters for equivalence motion_template_cursor at line " << line_index << "in the file '" << pathname << "'.");
                for (natural_32_bit  index : params)
                    output.push_back({keyword, index});
            },
            false,
            nullptr,
            &data
            );
}

keyframe_equivalences_data::~keyframe_equivalences_data()
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

                if (pose_frames.size() == 0U)
                    throw std::runtime_error("The 'pose_frames' is empty.");
                if (pose_frames.size() != names.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'names' are different.");
                if (pose_frames.size() != hierarchy.parents().size())
                    throw std::runtime_error("The size of 'pose_frames' and 'hierarchy.parents()' are different.");
                if (pose_frames.size() != lengths.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'lengths' are different.");

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
                    if (record.keyframe_equivalences.empty())
                        throw std::runtime_error(msgstream() << "The 'keyframe_equivalences' were not loaded to 'motions_map[" << entry.first << "]'.");

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
                    if (record.keyframes.num_keyframes() != record.keyframe_equivalences.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'keyframe_equivalences' differ in 'motions_map[" << entry.first << "]'.");
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
                        if (record.keyframe_equivalences.empty() && boost::filesystem::is_regular_file(animation_pathname / "meta_keyframe_equivalences.txt"))
                            record.keyframe_equivalences = meta::keyframe_equivalences(animation_pathname / "meta_keyframe_equivalences.txt", 1U, ultimate_finaliser);
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
