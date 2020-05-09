#ifndef AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED
#   define AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED

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


}}

namespace ai { namespace detail { namespace meta {


using  reference_frames = qtgl::modelspace;


}}}

namespace ai { namespace detail { namespace meta {


struct  mass_distribution
{
    float_32_bit  mass_inverted;
    matrix33  inertia_tensor_inverted;

    bool  operator==(mass_distribution const&  other) const
    {
        return this == &other || (are_equal(mass_inverted, other.mass_inverted, 0.0001f) &&
                                  are_equal_33(inertia_tensor_inverted, other.inertia_tensor_inverted, 0.0001f));
    }
    bool  operator!=(mass_distribution const&  other) const { return !(*this == other); }
};
using  mass_distribution_ptr = std::shared_ptr<mass_distribution const>;


struct  mass_distributions_data
{
    explicit mass_distributions_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~mass_distributions_data();

    std::vector<mass_distribution_ptr>  data;
};

struct  mass_distributions : public async::resource_accessor<mass_distributions_data>
{
    mass_distributions() : async::resource_accessor<mass_distributions_data>() {}
    mass_distributions(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<mass_distributions_data>(
    { "ai::skeletal_motion_templates::meta::mass_distributions",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<mass_distribution_ptr> const&  data() const { return resource().data; }
    mass_distribution_ptr  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}}

namespace ai { namespace detail { namespace meta {


struct  collider
{
    float_32_bit  weight;

    virtual  ~collider() {}
    bool  operator==(collider const&  other) const
    {
        if (this == &other) return true;
        if (!are_equal(weight, other.weight, 0.0001f)) return false;
        return typeid(*this) == typeid(other) && equals(other);
    }
    bool  operator!=(collider const&  other) const { return !(*this == other); }

protected:
    virtual bool  equals(collider const&  other) const = 0;
};
using  collider_ptr = std::shared_ptr<collider const>;

struct  collider_capsule : public collider
{
    float_32_bit  length;
    float_32_bit  radius;

    bool  equals(collider const&  other) const override { return *this == dynamic_cast<collider_capsule const&>(other); }
    bool  operator==(collider_capsule const&  other) const
    {
        return are_equal(length, other.length, 0.0001f) && are_equal(radius, other.radius, 0.0001f);
    }
};

struct  colliders_data
{
    explicit colliders_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~colliders_data();

    std::vector<collider_ptr>  data;
};

struct  colliders : public async::resource_accessor<colliders_data>
{
    colliders() : async::resource_accessor<colliders_data>() {}
    colliders(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<colliders_data>(
    { "ai::skeletal_motion_templates::meta::colliders",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<collider_ptr> const&  data() const { return resource().data; }
    collider_ptr  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}}

namespace ai { namespace detail { namespace meta {


struct  free_bones_for_look_at
{
	std::vector<natural_32_bit>  all_bones;
	std::vector<natural_32_bit>  end_effector_bones;

    bool  operator==(free_bones_for_look_at const&  other) const;
    bool  operator!=(free_bones_for_look_at const&  other) const { return !(*this == other); }
};
using  free_bones_for_look_at_ptr = std::shared_ptr<free_bones_for_look_at const>;

struct  free_bones_data
{
    explicit free_bones_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~free_bones_data();

    std::vector<free_bones_for_look_at_ptr>  look_at;
};

struct  free_bones : public async::resource_accessor<free_bones_data>
{
	free_bones() : async::resource_accessor<free_bones_data>() {}
	free_bones(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<free_bones_data>(
			{ "ai::skeletal_motion_templates::meta::free_bones",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<free_bones_for_look_at_ptr> const& look_at() const { return resource().look_at; }
    std::size_t size() const { return look_at().size(); }
};


}}}

namespace ai { namespace detail {


struct  bone_names_data
{
    explicit bone_names_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~bone_names_data();

    std::vector<std::string>  data;
};

struct  bone_names : public async::resource_accessor<bone_names_data>
{
    bone_names() : async::resource_accessor<bone_names_data>() {}
    bone_names(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<bone_names_data>(
    { "ai::skeletal_motion_templates::bone_names",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<std::string> const&  data() const { return resource().data; }
    std::string const&  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}

namespace ai { namespace detail {


struct  bone_hierarchy_data
{
    explicit bone_hierarchy_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~bone_hierarchy_data();

    // INVARIANT(parents.at(0)==-1); The first bone must be the 'pivot/root' bone of the skeleton, like 'hips'.
    std::vector<integer_32_bit>  parents;
    std::vector<std::vector<integer_32_bit> >  children;
};

struct  bone_hierarchy : public async::resource_accessor<bone_hierarchy_data>
{
    bone_hierarchy() : async::resource_accessor<bone_hierarchy_data>() {}
    bone_hierarchy(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<bone_hierarchy_data>(
    { "ai::skeletal_motion_templates::bone_hierarchy",path.string() },
            priority,
            parent_finaliser
            )
    {}

    // INVARIANT(parents.at(0)==-1); The first bone must be the 'pivot/root' bone of the skeleton, like 'hips'.
    std::vector<integer_32_bit> const&  parents() const { return resource().parents; }
    integer_32_bit  parent(natural_32_bit const  index) const { return parents().at(index); }

    std::vector<std::vector<integer_32_bit> > const&  children() const { return resource().children; }
    std::vector<integer_32_bit> const&  children_at(natural_32_bit const  index) const { return children().at(index); }
};


}}

namespace ai { namespace detail {


struct  bone_lengths_data
{
    explicit bone_lengths_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~bone_lengths_data();

    std::vector<float_32_bit>  data;
};

struct  bone_lengths : public async::resource_accessor<bone_lengths_data>
{
    bone_lengths() : async::resource_accessor<bone_lengths_data>() {}
    bone_lengths(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<bone_lengths_data>(
    { "ai::skeletal_motion_templates::bone_lengths",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<float_32_bit> const&  data() const { return resource().data; }
    float_32_bit  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}

namespace ai { namespace detail {


struct  bone_joints_data
{
    explicit bone_joints_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~bone_joints_data();

    std::vector<std::vector<angeo::joint_rotation_props> >  data;
};

struct  bone_joints : public async::resource_accessor<bone_joints_data>
{
	bone_joints() : async::resource_accessor<bone_joints_data>() {}
	bone_joints(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<bone_joints_data>(
			{ "ai::skeletal_motion_templates::bone_joints",path.string() },
            priority,
            parent_finaliser
            )
    {}
	std::vector<std::vector<angeo::joint_rotation_props> > const&  data() const { return resource().data; }
	std::vector<angeo::joint_rotation_props>  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}

namespace ai { namespace detail {


struct  anim_space_directions_data
{
    explicit anim_space_directions_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~anim_space_directions_data();

    vector3  forward;   // unit vector.
    vector3  up;        // unit vector.
};

struct  anim_space_directions : public async::resource_accessor<anim_space_directions_data>
{
    anim_space_directions() : async::resource_accessor<anim_space_directions_data>() {}
    anim_space_directions(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<anim_space_directions_data>(
    { "ai::skeletal_motion_templates::anim_space_directions",path.string() },
            priority,
            parent_finaliser
            )
    {}

    // Unit vector
    vector3 const&  forward() const { return resource().forward; }

    // Unit vector
    vector3 const&  up() const { return resource().up; }
};


}}

namespace ai { namespace detail {


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

    using  binding_info_vector = std::vector<std::pair<std::string, binding_info> >;
    using  transition_penalties_map = std::unordered_map<std::pair<std::string, std::string>, float_32_bit>;
    using  desire_to_speed_convertor_curves = std::vector<angeo::linear_segment_curve>;

    //explicit  motion_object_binding(boost::property_tree::ptree const&  root);

    binding_info_vector  binding_infos;
    transition_penalties_map  transition_penalties;
    desire_to_speed_convertor_curves  desire_to_speed_convertors;
};


}}

namespace ai { namespace detail {


using  keyframe = qtgl::keyframe;
using  keyframes = qtgl::keyframes;

struct  motion_template
{
    keyframes  keyframes;
    meta::reference_frames  reference_frames;
    meta::mass_distributions  mass_distributions;
    meta::colliders  colliders;
	meta::free_bones  free_bones;
};


}}

namespace ai { namespace detail {


struct  skeletal_motion_templates_data
{
    using  modelspace = qtgl::modelspace; 

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

    modelspace  pose_frames;
    bone_names  names;
    bone_hierarchy  hierarchy;
    bone_lengths  lengths;
	bone_joints  joints;

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

    using  keyframe = detail::keyframe;
    using  keyframes = detail::keyframes;
    using  modelspace = detail::skeletal_motion_templates_data::modelspace;

    using  transition_info = detail::skeletal_motion_templates_data::transition_info;
    using  transition = detail::skeletal_motion_templates_data::transition;
    using  transitions_map = detail::skeletal_motion_templates_data::transitions_map;
    using  loop_target = detail::skeletal_motion_templates_data::loop_target;
    using  motion_object_binding_map = detail::skeletal_motion_templates_data::motion_object_binding_map;
    using  motion_object_binding = detail::motion_object_binding;

    using  bone_names = detail::bone_names;
    using  bone_hierarchy = detail::bone_hierarchy;
    using  bone_lengths = detail::bone_lengths;
	using  bone_joints = detail::bone_joints;
	using  anim_space_directions = detail::anim_space_directions;

    using  motion_template = detail::motion_template;

    using  mass_distribution = detail::meta::mass_distribution;
    using  mass_distribution_ptr = detail::meta::mass_distribution_ptr;

    using  free_bones_for_look_at_ptr = detail::meta::free_bones_for_look_at_ptr;

    using  collider = detail::meta::collider;
    using  collider_ptr = detail::meta::collider_ptr;
    using  collider_capsule = detail::meta::collider_capsule;

    modelspace  pose_frames() const { return resource().pose_frames; }
    bone_names  names() const { return resource().names; }
    bone_hierarchy  hierarchy() const { return resource().hierarchy; }
    bone_lengths  lengths() const { return resource().lengths; }
	bone_joints  joints() const { return resource().joints; }

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
