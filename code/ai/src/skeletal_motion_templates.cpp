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
    , from_pose_matrices()
    , to_pose_matrices()
    , names()
    , hierarchy()
    , lengths()
    , joints()
    , look_at()
    , aim_at()
    , look_at_origin_bone(0U)
    , aim_at_origin_bone(0U)

    , motions_map()
    , transitions()
    , default_transition_props({ 0U, 0.0f })
{
    TMPROF_BLOCK();

    async::finalise_load_on_destroy_ptr const  ultimate_finaliser = async::finalise_load_on_destroy::create(
            [this](async::finalise_load_on_destroy_ptr) -> void {

                // All data are loaded. Let's apply post-load setup.

                from_pose_matrices.resize(pose_frames.size());
                to_pose_matrices.resize(pose_frames.size());
                for (natural_32_bit  bone = 0U; bone != pose_frames.size(); ++bone)
                {
                    angeo::from_base_matrix(pose_frames.at(bone), from_pose_matrices.at(bone));
                    angeo::to_base_matrix(pose_frames.at(bone), to_pose_matrices.at(bone));
                    if (hierarchy.parents.at(bone) >= 0)
                    {
                        to_pose_matrices.at(bone) = to_pose_matrices.at(bone) * to_pose_matrices.at(hierarchy.parents.at(bone));
                        from_pose_matrices.at(bone) = from_pose_matrices.at(hierarchy.parents.at(bone)) * from_pose_matrices.at(bone);
                    }
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
            look_at_origin_bone = ptree->get<natural_32_bit>("origin_bone");
            for (boost::property_tree::ptree::value_type const&  name_and_data : ptree->get_child("chains"))
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
            aim_at_origin_bone = ptree->get<natural_32_bit>("origin_bone");
            for (boost::property_tree::ptree::value_type const&  name_and_data : ptree->get_child("chains"))
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
