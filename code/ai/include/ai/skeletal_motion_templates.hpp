#ifndef AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED
#   define AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <angeo/tensor_math.hpp>
#	include <angeo/skeleton_kinematics.hpp>
#   include <gfx/keyframe.hpp>
#   include <gfx/modelspace.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <string>
#   include <vector>
#   include <memory>
#   include <functional>

namespace ai { namespace detail {


struct  motion_template_cursor
{
    std::string  motion_name;
    natural_32_bit  keyframe_index;

    struct  hasher { std::size_t operator()(motion_template_cursor const&  value) const; };

    bool  operator==(motion_template_cursor const&  other) const
    {
        return keyframe_index == other.keyframe_index && motion_name == other.motion_name;
    }
    bool  operator!=(motion_template_cursor const&  other) const { return !(*this == other); }
    bool  operator<(motion_template_cursor const&  other) const
    {
        return motion_name < other.motion_name || (motion_name == other.motion_name && keyframe_index < other.keyframe_index);
    }

    void  swap(motion_template_cursor&  other)
    {
        motion_name.swap(other.motion_name);
        std::swap(keyframe_index, other.keyframe_index);
    }
};


struct  motion_template
{
    using  keyframes_type = gfx::keyframes;
    using  reference_frames_type = gfx::modelspace;

    keyframes_type  keyframes;
    reference_frames_type  reference_frames;
    std::vector<vector3>  bboxes;   // Half sizes of bboxes along axes. They are expressed in corresponding 'reference_frames'.
};


struct  skeletal_motion_templates_data
{
    using  bone_names = std::vector<std::string>;
    using  bone_lengths = std::vector<float_32_bit>;
    using  bone_joints = angeo::joint_rotation_props_of_bones;

    struct  look_at_info
    {
        natural_32_bit  end_effector_bone;
        natural_32_bit  root_bone;
        std::unordered_set<natural_32_bit>  all_bones;
        vector3  direction;
    };
    using  look_at_info_ptr = std::shared_ptr<look_at_info const>;
    using  look_at_infos = std::unordered_map<std::string, look_at_info_ptr>;

    struct  aim_at_info
    {
        natural_32_bit  end_effector_bone;
        natural_32_bit  root_bone;
        std::unordered_set<natural_32_bit>  all_bones;
        std::unordered_map<std::string, vector3>  touch_points;
    };
    using  aim_at_info_ptr = std::shared_ptr<aim_at_info const>;
    using  aim_at_infos = std::unordered_map<std::string, aim_at_info_ptr>;

    struct  bone_hierarchy
    {
        using  parents_vector = std::vector<integer_32_bit>;
        using  children_vector = std::vector<std::vector<integer_32_bit> >;

        // INVARIANT(parents.at(0)==-1); The first bone must be the 'pivot/root' bone of the skeleton, like 'hips'.
        parents_vector  parents;
        children_vector  children;
    };

    struct  transition_info
    {
        std::array<natural_32_bit, 2U>  keyframe_indices;
        float_32_bit  seconds_to_interpolate;
    };

    struct  transition
    {
        bool  is_bidirectional;
        std::vector<transition_info>  transitions;
    };

    using  transitions_map = std::unordered_map<std::pair<std::string, std::string>, transition>;

    motion_template::reference_frames_type  pose_frames;
    std::vector<matrix44>  from_pose_matrices;  // I.e. matrices from spaces of pose bones to the motion tempalte space.
    std::vector<matrix44>  to_pose_matrices;    // I.e. matrices from the motion tempalte space to spaces of pose bones.
    bone_names  names;
    bone_hierarchy  hierarchy;
    bone_lengths  lengths;
	bone_joints  joints;
    look_at_infos  look_at;
    aim_at_infos  aim_at;
    natural_32_bit  look_at_origin_bone;
    natural_32_bit  aim_at_origin_bone;

    std::unordered_map<std::string, motion_template>  motions_map;
    transitions_map  transitions;
    std::pair<natural_32_bit, float_32_bit>  default_transition_props;

    explicit skeletal_motion_templates_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~skeletal_motion_templates_data();
};


}}

namespace ai {


struct  skeletal_motion_templates : public async::resource_accessor<detail::skeletal_motion_templates_data>
{
    skeletal_motion_templates()
        : async::resource_accessor<detail::skeletal_motion_templates_data>()
    {}

    skeletal_motion_templates(
            boost::filesystem::path const&  path,
            async::load_priority_type const  priority,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::skeletal_motion_templates_data>(
            {"ai::skeletal_motion_templates",path.string()},
            priority,
            parent_finaliser
            )
    {}

    using  motion_template_cursor = detail::motion_template_cursor;

    using  keyframes = detail::motion_template::keyframes_type;
    using  reference_frames = detail::motion_template::reference_frames_type;

    using  bone_names = detail::skeletal_motion_templates_data::bone_names;
    using  bone_hierarchy = detail::skeletal_motion_templates_data::bone_hierarchy;
    using  bone_lengths = detail::skeletal_motion_templates_data::bone_lengths;
    using  bone_joints = detail::skeletal_motion_templates_data::bone_joints;
    using  look_at_info_ptr = detail::skeletal_motion_templates_data::look_at_info_ptr;
    using  look_at_infos = detail::skeletal_motion_templates_data::look_at_infos;
    using  aim_at_info_ptr = detail::skeletal_motion_templates_data::aim_at_info_ptr;
    using  aim_at_infos = detail::skeletal_motion_templates_data::aim_at_infos;

    using  motion_template = detail::motion_template;

    using  transition_info = detail::skeletal_motion_templates_data::transition_info;
    using  transition = detail::skeletal_motion_templates_data::transition;
    using  transitions_map = detail::skeletal_motion_templates_data::transitions_map;

    reference_frames  pose_frames() const { return resource().pose_frames; }
    std::vector<matrix44> const&  from_pose_matrices() const { return resource().from_pose_matrices; }
    std::vector<matrix44> const&  to_pose_matrices() const { return resource().to_pose_matrices; }
    bone_names const&  names() const { return resource().names; }
    bone_hierarchy const&  hierarchy() const { return resource().hierarchy; }
    bone_hierarchy::parents_vector const&  parents() const { return hierarchy().parents; }
    bone_hierarchy::children_vector const&  children() const { return hierarchy().children; }
    bone_lengths const&  lengths() const { return resource().lengths; }
	bone_joints const&  joints() const { return resource().joints; }
    look_at_infos const&  look_at() const { return resource().look_at; }
    natural_32_bit  look_at_origin_bone() const { return resource().look_at_origin_bone; }
    aim_at_infos const&  aim_at() const { return resource().aim_at; }
    natural_32_bit  aim_at_origin_bone() const { return resource().aim_at_origin_bone; }

    std::unordered_map<std::string, motion_template> const&  motions_map() const { return resource().motions_map; }
    motion_template const&  at(std::string const&  motion_name) const { return motions_map().at(motion_name); }
    transitions_map const&  transitions() const { return resource().transitions; }
    std::pair<natural_32_bit, float_32_bit> const&  default_transition_props() const { return resource().default_transition_props; }
};


}

#endif
