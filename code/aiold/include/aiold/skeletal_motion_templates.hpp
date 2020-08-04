#ifndef AIOLD_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED
#   define AIOLD_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <angeo/tensor_math.hpp>
#	include <angeo/skeleton_kinematics.hpp>
#   include <angeo/linear_segment_curve.hpp>
#   include <qtgl/keyframe.hpp>
#   include <qtgl/modelspace.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <string>
#   include <vector>
#   include <memory>
#   include <functional>

namespace aiold { namespace detail {


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
    using  keyframes_type = qtgl::keyframes;
    using  reference_frames_type = qtgl::modelspace;
    using  aim_at_names = std::unordered_set<std::string>;

    keyframes_type  keyframes;
    reference_frames_type  reference_frames;
    std::vector<vector3>  bboxes;   // Half sizes of bboxes along axes. They are expressed in corresponding 'reference_frames'.
    std::vector<aim_at_names>  aim_at;
};


struct  motion_object_binding
{
    struct speed_sensitivity
    {
        float_32_bit  sum() const { return linear + angular; }

        // Both members must be in <0,1>; 0 means not sensitive at all.
        float_32_bit  linear;
        float_32_bit  angular;
    };

    struct  motion_match_def
    {
        angeo::linear_segment_curve  seconds_since_last_contact;
        angeo::linear_segment_curve  angle_to_straight_pose;
        vector3  ideal_linear_velocity_dir;
        angeo::linear_segment_curve  linear_velocity_dir;
        angeo::linear_segment_curve  linear_velocity_mag;
        angeo::linear_segment_curve  angular_speed;
    };


    struct  desire_match_def
    {
        angeo::linear_segment_curve  speed_forward;
        angeo::linear_segment_curve  speed_left;
        angeo::linear_segment_curve  speed_up;
        angeo::linear_segment_curve  speed_turn_ccw;
    };


    struct  binding_info
    {
        speed_sensitivity  speedup_sensitivity;
        motion_match_def  motion_matcher;
        desire_match_def  desire_matcher;
        bool  disable_upper_joint;
    };

    struct  desire_values_intepretation_curves
    {
        struct  move_curves
        {
            angeo::linear_segment_curve  forward;
            angeo::linear_segment_curve  left;
            angeo::linear_segment_curve  up;
            angeo::linear_segment_curve  turn_ccw;
        };

        struct  guesture_curves
        {
            struct  subject_curves
            {
                angeo::linear_segment_curve  head;
                angeo::linear_segment_curve  tail;
            };

            struct  sign_curves
            {
                angeo::linear_segment_curve  head;
                angeo::linear_segment_curve  tail;
                angeo::linear_segment_curve  intensity;
            };

            subject_curves  subject;
            sign_curves  sign;
        };

        struct  target_curves
        {
            angeo::linear_segment_curve  longitude;
            angeo::linear_segment_curve  altitude;
            angeo::linear_segment_curve  magnitude;
        };

        move_curves  move;
        guesture_curves  guesture;
        target_curves  look_at;
        target_curves  aim_at;
    };

    using  binding_info_vector = std::vector<std::pair<std::string, binding_info> >;
    using  transition_penalties_map = std::unordered_map<std::pair<std::string, std::string>, float_32_bit>;

    //explicit  motion_object_binding(boost::property_tree::ptree const&  root);

    binding_info_vector  binding_infos;
    transition_penalties_map  transition_penalties;
    desire_values_intepretation_curves  desire_values_interpreters;
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

    struct  loop_target
    {
        natural_32_bit  index;
        float_32_bit  seconds_to_interpolate;
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

    using  motion_object_binding_map = std::unordered_map<std::string, detail::motion_object_binding>;

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
    std::unordered_map<std::string, loop_target>  loop_targets;
    motion_template_cursor  initial_motion_template;
    transitions_map  transitions;
    std::pair<natural_32_bit, float_32_bit>  default_transition_props;
    motion_object_binding_map  motion_object_bindings;

    explicit skeletal_motion_templates_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~skeletal_motion_templates_data();
};


}}

namespace aiold {


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
            {"aiold::skeletal_motion_templates",path.string()},
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
    using  loop_target = detail::skeletal_motion_templates_data::loop_target;
    using  motion_object_binding_map = detail::skeletal_motion_templates_data::motion_object_binding_map;
    using  motion_object_binding = detail::motion_object_binding;

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
    std::unordered_map<std::string, loop_target> const&  loop_targets() const { return resource().loop_targets; };
    motion_template_cursor const&  initial_motion_template() const { return resource().initial_motion_template; }
    transitions_map const&  transitions() const { return resource().transitions; }
    std::pair<natural_32_bit, float_32_bit> const&  default_transition_props() const { return resource().default_transition_props; }
    motion_object_binding_map const&  motion_object_bindings() const { return resource().motion_object_bindings; }
};


}

#endif