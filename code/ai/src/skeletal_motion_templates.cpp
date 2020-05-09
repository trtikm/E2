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


std::unique_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname)
{
    std::unique_ptr<boost::property_tree::ptree>  ptree(new boost::property_tree::ptree);
    if (!boost::filesystem::is_regular_file(ptree_pathname))
        throw std::runtime_error(msgstream() << "Cannot access the file '" << ptree_pathname << "'.");
    boost::property_tree::read_info(ptree_pathname.string(), *ptree);
    return std::move(ptree);
}


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


skeletal_motion_templates_data::skeletal_motion_templates_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : pose_frames()
    , names()
    , hierarchy()
    , lengths()
    , joints()

    , motions_map()
    , loop_targets()
    , initial_motion_template()
    , transitions()
    , default_transition_props()
    , motion_object_bindings()
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
                if (joints.empty())
                    throw std::runtime_error("The 'joints' is empty.");
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
                    if (record.keyframes.num_keyframes() != record.free_bones.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'free_bones' differ in 'motions_map[" << entry.first << "]'.");
                }
            },
            finaliser
            );

    auto const  load_linear_segment_function = [](boost::property_tree::ptree const&  ptree, std::vector<vector2>&  points) {
        for (boost::property_tree::ptree::value_type const&  void_and_data : ptree)
            points.push_back({ void_and_data.second.get<float_32_bit>("x") , void_and_data.second.get<float_32_bit>("y") });
    };

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
        if (joints.empty() && boost::filesystem::is_regular_file(pathname / "joints.txt"))
            joints = bone_joints(pathname / "joints.txt", 10U, ultimate_finaliser);

        if (boost::filesystem::is_regular_file(pathname / "loop_targets.txt"))
        {
            std::unique_ptr<boost::property_tree::ptree> const  ptree = load_ptree(pathname / "loop_targets.txt");
            for (boost::property_tree::ptree::value_type const&  name_and_props : *ptree)
            {
                auto const  it = loop_targets.find(name_and_props.first);
                if (it != loop_targets.cend())
                    continue;
                auto&  props = loop_targets[name_and_props.first];
                props.index = name_and_props.second.get<natural_32_bit>("index");
                props.seconds_to_interpolate = name_and_props.second.get<float_32_bit>("seconds_to_interpolate");
            }
        }
        if (initial_motion_template.motion_name.empty() && boost::filesystem::is_regular_file(pathname / "initial_motion_template.txt"))
        {
            std::ifstream  istr;
            open_file_stream_for_reading(istr, pathname / "initial_motion_template.txt");
            initial_motion_template.motion_name = read_line(istr);
            std::string const index_string = read_line(istr);
            initial_motion_template.keyframe_index = std::atoi(index_string.c_str());
            istr.close();
        }
        if (boost::filesystem::is_regular_file(pathname / "transitions.txt"))
        {
            std::unique_ptr<boost::property_tree::ptree> const  ptree = load_ptree(pathname / "transitions.txt");
            default_transition_props.first = ptree->get<natural_32_bit>("default_transition_props.index");
            default_transition_props.second = ptree->get<float_32_bit>("default_transition_props.seconds_to_interpolate");
            for (boost::property_tree::ptree::value_type const&  void_and_props : ptree->get_child("transitions"))
            {
                std::pair<std::string, std::string> const  key{
                        void_and_props.second.get<std::string>("key1"),
                        void_and_props.second.get<std::string>("key2")
                        };
                auto const  it = transitions.find(key);
                if (it != transitions.cend())
                    continue;
                auto&  props = transitions[key];
                props.is_bidirectional = void_and_props.second.get<bool>("is_bidirectional");
                for (boost::property_tree::ptree::value_type const&  void_and_transition : void_and_props.second.get_child("transitions"))
                    props.transitions.push_back({
                            {
                                void_and_transition.second.get<natural_32_bit>("index1"),
                                void_and_transition.second.get<natural_32_bit>("index2"),
                            },
                            void_and_transition.second.get<float_32_bit>("seconds_to_interpolate"),
                            });
            }
        }

        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(pathname))
        {
            if (!boost::filesystem::is_directory(entry.path()))
                continue;

            boost::filesystem::path const  entry_pathname = canonical_path(entry.path());
            std::string const  dir_name = entry_pathname.filename().string();
            if (dir_name == "!meta")
            {
                if (boost::filesystem::is_directory(entry_pathname / "motion_object_bindings"))
                {
                    for (boost::filesystem::directory_entry&  mob_entry :
                         boost::filesystem::directory_iterator(entry_pathname / "motion_object_bindings"))
                    {
                        if (!boost::filesystem::is_directory(mob_entry.path()))
                            continue;
                        boost::filesystem::path const  mob_pathname = canonical_path(mob_entry.path());
                        std::string const  motion_object_name = mob_pathname.filename().string();

                        auto const  it = motion_object_bindings.find(motion_object_name);
                        if (it != motion_object_bindings.cend())
                            continue;
                        auto& binding = motion_object_bindings[motion_object_name];

                        if (boost::filesystem::is_directory(mob_pathname / "binding_infos"))
                            for (boost::filesystem::directory_entry&  bi_entry :
                                 boost::filesystem::directory_iterator(mob_pathname / "binding_infos"))
                            {
                                if (!boost::filesystem::is_regular_file(bi_entry.path()))
                                    continue;
                                boost::filesystem::path const  bi_pathname = canonical_path(bi_entry.path());
                                std::string const  bi_file_name = bi_pathname.filename().string();
                                if (bi_file_name.size() < 5U || !boost::ends_with(bi_file_name, ".txt"))
                                    throw std::runtime_error(msgstream() << "Wrong binding_info file name: " << bi_pathname);
                                std::unique_ptr<boost::property_tree::ptree> const  bi_ptree = load_ptree(bi_pathname);
                                binding.binding_infos.push_back({ bi_file_name.substr(0, bi_file_name.size() - 4U), {} });
                                motion_object_binding::binding_info&  info = binding.binding_infos.back().second;
                                info.speedup_sensitivity.linear = bi_ptree->get<float_32_bit>("speedup_sensitivity.linear");
                                info.speedup_sensitivity.angular = bi_ptree->get<float_32_bit>("speedup_sensitivity.angular");
                                load_linear_segment_function(
                                        bi_ptree->get_child("motion_matcher.seconds_since_last_contact"),
                                        info.motion_matcher.seconds_since_last_contact.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("motion_matcher.angle_to_straight_pose"),
                                        info.motion_matcher.angle_to_straight_pose.points
                                        );
                                info.motion_matcher.ideal_linear_velocity_dir = {
                                        bi_ptree->get<float_32_bit>("motion_matcher.ideal_linear_velocity_dir.x"),
                                        bi_ptree->get<float_32_bit>("motion_matcher.ideal_linear_velocity_dir.y"),
                                        bi_ptree->get<float_32_bit>("motion_matcher.ideal_linear_velocity_dir.z")
                                        };
                                load_linear_segment_function(
                                        bi_ptree->get_child("motion_matcher.linear_velocity_dir"),
                                        info.motion_matcher.linear_velocity_dir.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("motion_matcher.linear_velocity_mag"),
                                        info.motion_matcher.linear_velocity_mag.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("motion_matcher.angular_speed"),
                                        info.motion_matcher.angular_speed.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("desire_matcher.speed_forward"),
                                        info.desire_matcher.speed_forward.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("desire_matcher.speed_left"),
                                        info.desire_matcher.speed_left.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("desire_matcher.speed_up"),
                                        info.desire_matcher.speed_up.points
                                        );
                                load_linear_segment_function(
                                        bi_ptree->get_child("desire_matcher.speed_turn_ccw"),
                                        info.desire_matcher.speed_turn_ccw.points
                                        );
                                info.disable_upper_joint = bi_ptree->get<bool>("disable_upper_joint");
                            }
                        if (boost::filesystem::is_regular_file(mob_pathname / "desire_to_speed_convertors.txt"))
                        {
                            std::unique_ptr<boost::property_tree::ptree> const  desire_to_speed_convertors_ptree =
                                    load_ptree(mob_pathname / "desire_to_speed_convertors.txt");
                            binding.desire_to_speed_convertors.resize(4U);
                            load_linear_segment_function(
                                    desire_to_speed_convertors_ptree->get_child("FORWARD"),
                                    binding.desire_to_speed_convertors.at(DESIRE_COORD::FORWARD).points
                                    );
                            load_linear_segment_function(
                                    desire_to_speed_convertors_ptree->get_child("LEFT"),
                                    binding.desire_to_speed_convertors.at(DESIRE_COORD::LEFT).points
                                    );
                            load_linear_segment_function(
                                    desire_to_speed_convertors_ptree->get_child("UP"),
                                    binding.desire_to_speed_convertors.at(DESIRE_COORD::UP).points
                                    );
                            load_linear_segment_function(
                                    desire_to_speed_convertors_ptree->get_child("TURN_CCW"),
                                    binding.desire_to_speed_convertors.at(DESIRE_COORD::TURN_CCW).points
                                    );
                        }
                        if (boost::filesystem::is_regular_file(mob_pathname / "transition_penalties.txt"))
                        {
                            std::unique_ptr<boost::property_tree::ptree> const  transition_penalties_ptree =
                                    load_ptree(mob_pathname / "transition_penalties.txt");
                            for (boost::property_tree::ptree::value_type const&  void_and_props : *transition_penalties_ptree)
                                binding.transition_penalties.insert({
                                    {
                                        void_and_props.second.get<std::string>("from"),
                                        void_and_props.second.get<std::string>("to")
                                    },
                                    void_and_props.second.get<float_32_bit>("penalty")
                                    });
                        }
                    }
                }
            }
            else
            {
                motion_template&  record = motions_map[dir_name];
                bool  has_keyframes = false;
                for (boost::filesystem::directory_entry& animation_entry : boost::filesystem::directory_iterator(entry_pathname))
                    if (boost::filesystem::is_regular_file(animation_entry.path()))
                    {
                        if (has_keyframes == false)
                        {
                            std::string const  filename = animation_entry.path().filename().string();
                            std::string const  extension = animation_entry.path().filename().extension().string();
                            if (filename.find("keyframe") == 0UL && extension == ".txt")
                                has_keyframes = true;
                        }

                        if (record.reference_frames.empty() && boost::filesystem::is_regular_file(entry_pathname / "meta_reference_frames.txt"))
                            record.reference_frames = meta::reference_frames(entry_pathname / "meta_reference_frames.txt", 1U, ultimate_finaliser, "ai::skeletal_motion_templates_data::reference_frames");
                        if (record.mass_distributions.empty() && boost::filesystem::is_regular_file(entry_pathname / "meta_mass_distributions.txt"))
                            record.mass_distributions = meta::mass_distributions(entry_pathname / "meta_mass_distributions.txt", 1U, ultimate_finaliser);
                        if (record.colliders.empty() && boost::filesystem::is_regular_file(entry_pathname / "meta_motion_colliders.txt"))
                            record.colliders = meta::colliders(entry_pathname / "meta_motion_colliders.txt", 1U, ultimate_finaliser);
                        if (record.free_bones.empty() && boost::filesystem::is_regular_file(entry_pathname / "meta_free_bones.txt"))
                            record.free_bones = meta::free_bones(entry_pathname / "meta_free_bones.txt", 1U, ultimate_finaliser);
                    }
                if (has_keyframes)
                    record.keyframes = keyframes(entry_pathname, 1U, ultimate_finaliser);
            }
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
