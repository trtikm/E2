#include <ai/skeletal_motion_templates.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/cortex.hpp>
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


std::unique_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname)
{
    std::unique_ptr<boost::property_tree::ptree>  ptree(new boost::property_tree::ptree);
    if (!boost::filesystem::is_regular_file(ptree_pathname))
        throw std::runtime_error(msgstream() << "Cannot access the file '" << ptree_pathname << "'.");
    boost::property_tree::read_info(ptree_pathname.string(), *ptree);
    return std::move(ptree);
}


}}}

namespace ai { namespace detail {


std::size_t  motion_template_cursor::hasher::operator()(motion_template_cursor const&  value) const
{
    std::size_t seed = 0;
    ::hash_combine(seed, value.motion_name);
    ::hash_combine(seed, value.keyframe_index);
    return seed;
}


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

                // All data are loaded. Let's apply post-load setup.
            
                for (auto&  entry : motions_map)
                {
                    motion_template&  record = entry.second;
                    if ((natural_32_bit)record.aim_at.size() < record.keyframes.num_keyframes())
                        record.aim_at.resize(record.keyframes.num_keyframes());
                }

                // And now let's check for consistency of loaded data.

                if (pose_frames.empty())
                    throw std::runtime_error("The 'pose_frames' were not loaded.");
                if (names.empty())
                    throw std::runtime_error("The 'names' were not loaded.");
                if (hierarchy.parents.empty())
                    throw std::runtime_error("The 'hierarchy' was not loaded.");
                if (lengths.empty())
                    throw std::runtime_error("The 'lengths' were not loaded.");
                if (motions_map.empty())
                    throw std::runtime_error("The 'motions_map' is empty.");

                if (pose_frames.size() == 0U)
                    throw std::runtime_error("The 'pose_frames' is empty.");
                if (pose_frames.size() != names.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'names' are different.");
                if (pose_frames.size() != hierarchy.parents.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'hierarchy.parents' are different.");
                if (pose_frames.size() != lengths.size())
                    throw std::runtime_error("The size of 'pose_frames' and 'lengths' are different.");

                if (hierarchy.parents.at(0U) != -1)
                    throw std::runtime_error("Invariant failure: hierarchy.parent(0) != -1.");

                for (auto const&  entry : motions_map)
                {
                    motion_template const&  record = entry.second;

                    if (record.keyframes.empty())
                        throw std::runtime_error(msgstream() << "The 'keyframes' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.reference_frames.empty())
                        throw std::runtime_error(msgstream() << "The 'reference_frames' were not loaded to 'motions_map[" << entry.first << "]'.");
                    if (record.bboxes.empty())
                        throw std::runtime_error(msgstream() << "The 'bboxes' were not loaded to 'motions_map[" << entry.first << "]'.");

                    if (record.keyframes.num_keyframes() < 2U)
                        throw std::runtime_error(msgstream() << "The count of keyframes is less than 2 in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.reference_frames.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'reference_frames' differ in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.bboxes.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'bboxes' differ in 'motions_map[" << entry.first << "]'.");
                    if (record.keyframes.num_keyframes() != record.aim_at.size())
                        throw std::runtime_error(msgstream() << "The count of keyframes and 'aim_at' differ in 'motions_map[" << entry.first << "]'.");
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
            angeo::open_file_stream_for_reading(istr, pathname / "imports.txt");
            for (std::string  line = read_line(istr); !line.empty(); line = read_line(istr))
            {
                boost::algorithm::trim(line);
                if (!line.empty())
                    work_list.push_back(canonical_path(pathname / line));
            }
        }

        if (pose_frames.empty() && boost::filesystem::is_regular_file(pathname / "pose.txt"))
            pose_frames = motion_template::reference_frames_type(pathname / "pose.txt", 10U, ultimate_finaliser);

        if (names.empty() && boost::filesystem::is_regular_file(pathname / "names.txt"))
        {
            boost::filesystem::path const  names_pathname = pathname / "names.txt";

            std::ifstream  istr;
            angeo::open_file_stream_for_reading(istr, names_pathname);

            std::unordered_set<std::string>  visited;
            for (natural_32_bit i = 0U, num_names = angeo::read_num_records(istr, names_pathname); i != num_names; ++i)
            {
                std::string  line;
                if (!read_line(istr, line))
                    throw std::runtime_error(msgstream() << "Cannot read name #" << i << " in the file '" << names_pathname << "'.");
                boost::algorithm::trim(line);

                if (visited.count(line) != 0UL)
                    throw std::runtime_error(msgstream() << "Duplicate bone name '" << line << "' at line #" << i << " in the file '" << names_pathname << "'.");
                visited.insert(line);

                names.push_back(line);
            }
        }

        if (hierarchy.parents.empty() && boost::filesystem::is_regular_file(pathname / "parents.txt"))
        {
            boost::filesystem::path const  parents_pathname = pathname / "parents.txt";

            std::ifstream  istr;
            angeo::open_file_stream_for_reading(istr, parents_pathname);

            std::unordered_set<std::string>  visited;
            for (natural_32_bit i = 0U, n = angeo::read_num_records(istr, parents_pathname); i != n; ++i)
            {
                std::string  line;
                if (!read_line(istr, line))
                    throw std::runtime_error(msgstream() << "Cannot read parent #" << i << " in the file '" << parents_pathname << "'.");
                integer_32_bit  parent_index;
                {
                    std::istringstream sstr(line);
                    sstr >> parent_index;
                }
                if (parent_index < -1)
                    throw std::runtime_error(msgstream() << "Wrong parent bone index " << parent_index << " for bone #" << i << " in the file '"
                                                         << parents_pathname << "'. E.i. the bones were not saved in the topological order.");
                if (parent_index >(integer_32_bit)i)
                    throw std::runtime_error(msgstream() << "Bone #" << i << " comes before its parent bone #" << parent_index << " in the file '"
                                                         << parents_pathname << "'. E.i. the bones were not saved in the topological order.");
                hierarchy.parents.push_back(parent_index);
            }

            angeo::skeleton_compute_child_bones(hierarchy.parents, hierarchy.children);
        }

        if (lengths.empty() && boost::filesystem::is_regular_file(pathname / "lengths.txt"))
        {
            boost::filesystem::path const  lengths_pathname = pathname / "lengths.txt";
            std::ifstream  istr;
            angeo::open_file_stream_for_reading(istr, lengths_pathname);

            for (natural_32_bit i = 0U, n = angeo::read_num_records(istr, lengths_pathname); i != n; ++i)
            {
                std::string  line;
                if (!read_line(istr, line))
                    throw std::runtime_error(msgstream() << "Cannot read length #" << i << " in the file '" << lengths_pathname << "'.");
                boost::algorithm::trim(line);

                float_32_bit  length;
                {
                    std::istringstream sstr(line);
                    sstr >> length;
                }
                if (length < 0.001f)
                    throw std::runtime_error(msgstream() << "Wrong value " << length << " for length #" << i << " in the file '" << lengths_pathname << "'.");

                lengths.push_back(length);
            }
        }

        if (joints.empty() && boost::filesystem::is_regular_file(pathname / "joints.txt"))
        {
            boost::filesystem::path const  joints_pathname = pathname / "joints.txt";
            
            std::unique_ptr<boost::property_tree::ptree> const  ptree = load_ptree(joints_pathname);
	        for (boost::property_tree::ptree::value_type const&  void_and_bone_joints : *ptree)
            {
                natural_32_bit const  bone = void_and_bone_joints.second.get<natural_32_bit>("bone_index");
                for (boost::property_tree::ptree::value_type const&  void_and_props :
                        void_and_bone_joints.second.get_child("joint_rotation_props"))
		        {
			        angeo::joint_rotation_props  joint;
			        joint.axis = vector3(void_and_props.second.get<float_32_bit>("axis.x"),
                                         void_and_props.second.get<float_32_bit>("axis.y"),
                                         void_and_props.second.get<float_32_bit>("axis.z"));
			        joint.zero_angle_direction = vector3(void_and_props.second.get<float_32_bit>("zero_angle_direction.x"),
                                                         void_and_props.second.get<float_32_bit>("zero_angle_direction.y"),
                                                         void_and_props.second.get<float_32_bit>("zero_angle_direction.z"));
			        joint.direction = vector3(void_and_props.second.get<float_32_bit>("direction.x"),
                                              void_and_props.second.get<float_32_bit>("direction.y"),
                                              void_and_props.second.get<float_32_bit>("direction.z"));
			        joint.max_angle = void_and_props.second.get<float_32_bit>("max_angle");
			        joint.max_angular_speed = void_and_props.second.get<float_32_bit>("max_angular_speed");
			        joints[bone].push_back(joint);
		        }
            }
        }

        if (boost::filesystem::is_regular_file(pathname / "look_at.txt"))
        {
            std::unique_ptr<boost::property_tree::ptree> const  ptree = load_ptree(pathname / "look_at.txt");
            for (boost::property_tree::ptree::value_type const&  name_and_data : *ptree)
            {
                std::shared_ptr<look_at_info>  info = std::make_shared<look_at_info>();
                for (boost::property_tree::ptree::value_type const&  void_and_bone : name_and_data.second.get_child("bones"))
                    info->all_bones.insert(void_and_bone.second.get_value<natural_32_bit>());
                info->end_effector_bone = name_and_data.second.get<natural_32_bit>("end_effector_bone");
                info->root_bone = name_and_data.second.get<natural_32_bit>("root_bone");
                info->direction = {
                        name_and_data.second.get<float_32_bit>("direction.x"),
                        name_and_data.second.get<float_32_bit>("direction.y"),
                        name_and_data.second.get<float_32_bit>("direction.z")
                        };
                look_at[name_and_data.first] = info;
            }
        }

        if (boost::filesystem::is_regular_file(pathname / "aim_at.txt"))
        {
            std::unique_ptr<boost::property_tree::ptree> const  ptree = load_ptree(pathname / "aim_at.txt");
            for (boost::property_tree::ptree::value_type const&  name_and_data : *ptree)
            {
                std::shared_ptr<aim_at_info>  info = std::make_shared<aim_at_info>();
                for (boost::property_tree::ptree::value_type const&  void_and_bone : name_and_data.second.get_child("bones"))
                    info->all_bones.insert(void_and_bone.second.get_value<natural_32_bit>());
                info->end_effector_bone = name_and_data.second.get<natural_32_bit>("end_effector_bone");
                info->root_bone = name_and_data.second.get<natural_32_bit>("root_bone");
                for (boost::property_tree::ptree::value_type const&  name_and_point : name_and_data.second.get_child("touch_points"))
                    info->touch_points.insert({
                            name_and_point.first,
                            vector3{
                                name_and_point.second.get<float_32_bit>("x"),
                                name_and_point.second.get<float_32_bit>("y"),
                                name_and_point.second.get<float_32_bit>("z")
                            }});
                aim_at[name_and_data.first] = info;
            }
        }

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
            angeo::open_file_stream_for_reading(istr, pathname / "initial_motion_template.txt");
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

                if (boost::filesystem::is_regular_file(entry_pathname / "keyframe0.txt"))
                    record.keyframes = motion_template::keyframes_type(entry_pathname, 1U, ultimate_finaliser);

                if (boost::filesystem::is_directory(entry_pathname / "!meta"))
                {
                    boost::filesystem::path const  meta_pathname = entry_pathname / "!meta";

                    if (record.reference_frames.empty() && boost::filesystem::is_regular_file(meta_pathname / "reference_frames.txt"))
                        record.reference_frames = motion_template::reference_frames_type(meta_pathname / "reference_frames.txt", 1U, ultimate_finaliser, "ai::skeletal_motion_templates_data::reference_frames");

                    if (record.bboxes.empty() && boost::filesystem::is_regular_file(meta_pathname / "bboxes.txt"))
                    {
                        boost::filesystem::path const  bboxes_pathname = meta_pathname / "bboxes.txt";
                        std::ifstream  istr;
                        angeo::open_file_stream_for_reading(istr, bboxes_pathname);
                        natural_32_bit  num_bboxes = angeo::read_num_records(istr, bboxes_pathname);
                        for (natural_32_bit i = 0U, line_number = 0U; i != num_bboxes; ++i)
                        {
                            vector3  sizes;
                            line_number = angeo::read_vector3(sizes, istr, line_number, pathname);
                            record.bboxes.push_back(sizes);
                        }
                        istr.close();
                    }

                    if (boost::filesystem::is_regular_file(meta_pathname / "aim_at.txt"))
                    {
                        std::unique_ptr<boost::property_tree::ptree> const  ptree = load_ptree(meta_pathname / "aim_at.txt");
                        for (boost::property_tree::ptree::value_type const&  void_and_data : *ptree)
                        {
                            std::string const  name = void_and_data.second.get<std::string>("name");
                            for (boost::property_tree::ptree::value_type const& void_and_range : void_and_data.second.get_child("keyframe_ranges"))
                            {
                                natural_32_bit const  n = void_and_range.second.get<natural_32_bit>("last");
                                if (n >= (natural_32_bit)record.aim_at.size())
                                    record.aim_at.resize(n + 1U);
                                for (natural_32_bit  i = void_and_range.second.get<natural_32_bit>("first"); i <= n; ++i)
                                    record.aim_at.at(i).insert(name);
                            }
                        }
                    }
                }
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
