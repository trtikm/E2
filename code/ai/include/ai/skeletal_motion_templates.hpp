#ifndef AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED
#   define AI_SKELETAL_MOTION_TEMPLATES_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/keyframe.hpp>
#   include <qtgl/modelspace.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <string>
#   include <vector>
#   include <memory>

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


struct  constraint
{
    virtual ~constraint() = 0 {}
    bool  operator==(constraint const&  other) const
    {
        if (this == &other) return true;
        return typeid(*this) == typeid(other) && equals(other);
    }
    bool  operator!=(constraint const&  other) const { return !(*this == other); }
protected:
    virtual bool  equals(constraint const&  other) const { return false; }
};
using  constraint_ptr = std::shared_ptr<constraint const>;


struct  constraints_data
{
    explicit constraints_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~constraints_data();

    std::vector<std::vector<constraint_ptr> >  data;
};

struct  constraints : public async::resource_accessor<constraints_data>
{
    constraints() : async::resource_accessor<constraints_data>() {}
    constraints(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<constraints_data>(
    { "ai::skeletal_motion_templates::meta::constraints",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<std::vector<constraint_ptr> > const&  data() const { return resource().data; }
    std::vector<constraint_ptr> const&  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}}

namespace ai { namespace detail { namespace meta {


struct  action
{
    virtual ~action() = 0 {}
    bool  operator==(action const&  other) const
    {
        if (this == &other) return true;
        return typeid(*this) == typeid(other) && equals(other);
    }
    bool  operator!=(action const&  other) const { return !(*this == other); }
protected:
    virtual bool  equals(action const&  other) const { return false; }
};
using  action_ptr = std::shared_ptr<action const>;


struct  actions_data
{
    explicit actions_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~actions_data();

    std::vector<std::vector<action_ptr> >  data;
};

struct  actions : public async::resource_accessor<actions_data>
{
    actions() : async::resource_accessor<actions_data>() {}
    actions(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<actions_data>(
    { "ai::skeletal_motion_templates::meta::actions",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<std::vector<action_ptr> > const&  data() const { return resource().data; }
    std::vector<action_ptr> const&  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
};


}}}

namespace ai { namespace detail { namespace meta {


struct  keyframe_equivalences_data
{
    explicit keyframe_equivalences_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~keyframe_equivalences_data();

    std::vector<std::vector<motion_template_cursor> >  data;
};

struct  keyframe_equivalences : public async::resource_accessor<keyframe_equivalences_data>
{
    keyframe_equivalences() : async::resource_accessor<keyframe_equivalences_data>() {}
    keyframe_equivalences(
        boost::filesystem::path const&  path,
        async::load_priority_type const  priority,
        async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
        : async::resource_accessor<keyframe_equivalences_data>(
    { "ai::skeletal_motion_templates::meta::keyframe_equivalences",path.string() },
            priority,
            parent_finaliser
            )
    {}
    std::vector<std::vector<motion_template_cursor> > const&  data() const { return resource().data; }
    std::vector<motion_template_cursor> const&  at(natural_32_bit const  index) const { return data().at(index); }
    std::size_t size() const { return data().size(); }
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


struct  skeletal_motion_templates_data
{
    using  keyframe = qtgl::keyframe;
    using  keyframes = qtgl::keyframes;
    using  modelspace = qtgl::modelspace; 

    struct  motion_template
    {
        keyframes  keyframes;
        meta::reference_frames  reference_frames;
        meta::mass_distributions  mass_distributions;
        meta::colliders  colliders;
        meta::constraints  constraints;
        meta::actions  actions;
        meta::keyframe_equivalences  keyframe_equivalences;
    };

    modelspace  pose_frames;
    bone_names  names;
    bone_hierarchy  hierarchy;
    bone_lengths  lengths;

    // std::vector<std::vector<angeo::joint_rotation_props> > rotation_props;     // NOT USED YET!

    // NOTE: 'anim' space is the space in which pose bones without a parent are defined. In other words, it is the common frame of reference
    //       of coord. systems of all pose bones without parent bones. Note also that all keyframes are defined in this 'anim' space.
    // NOTE: The two vectors below should also represent forward and up directions in each meta-reference-frame of each keyframe.
    anim_space_directions  directions;

    std::unordered_map<std::string, motion_template>  motions_map;

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

    struct  template_motion_info
    {
        motion_template_cursor  src_pose;
        motion_template_cursor  dst_pose;
        float_32_bit  total_interpolation_time_in_seconds;
        float_32_bit  consumed_time_in_seconds;
    };

    using  keyframe = detail::skeletal_motion_templates_data::keyframe;
    using  keyframes = detail::skeletal_motion_templates_data::keyframes;
    using  modelspace = detail::skeletal_motion_templates_data::modelspace;

    using  bone_names = detail::bone_names;
    using  bone_hierarchy = detail::bone_hierarchy;
    using  bone_lengths = detail::bone_lengths;
    using  anim_space_directions = detail::anim_space_directions;

    using  motion_template = detail::skeletal_motion_templates_data::motion_template;

    using  mass_distribution = detail::meta::mass_distribution;

    using  constraint = detail::meta::constraint;
    using  constraint_ptr = detail::meta::constraint_ptr;

    struct  constraint_contact_normal_cone : public constraint
    {
        vector3  unit_axis;
        float_32_bit  angle_in_radians;

        bool  equals(constraint const&  other) const override { return *this == dynamic_cast<constraint_contact_normal_cone const&>(other); }
        bool  operator==(constraint_contact_normal_cone const&  other) const
        {
            return are_equal_3d(unit_axis, other.unit_axis, 0.0001f) && are_equal(angle_in_radians, other.angle_in_radians, 0.0001f);
        }
    };

    using  action = detail::meta::action;
    using  action_ptr = detail::meta::action_ptr;

    struct  action_chase_ideal_linear_velocity : public action
    {
        float_32_bit  max_linear_accel;
        float_32_bit  motion_error_multiplier;
    };

    struct  action_chase_linear_velocity_by_forward_vector : public action
    {
        float_32_bit  max_angular_speed;
        float_32_bit  max_angular_accel;
        float_32_bit  min_linear_speed;
    };

    struct  action_turn_around : public action
    {
        float_32_bit  max_angular_accel;

        bool  equals(action const&  other) const override { return *this == dynamic_cast<action_turn_around const&>(other); }
        bool  operator==(action_turn_around const&  other) const { return are_equal(max_angular_accel, other.max_angular_accel, 0.0001f); }
    };

    struct  action_dont_move : public action
    {
        float_32_bit  max_linear_accel;

        bool  equals(action const&  other) const override { return *this == dynamic_cast<action_dont_move const&>(other); }
        bool  operator==(action_dont_move const&  other) const { return are_equal(max_linear_accel, other.max_linear_accel, 0.0001f); }
    };

    struct  action_dont_rotate : public action
    {
        float_32_bit  max_angular_accel;

        bool  equals(action const&  other) const override { return *this == dynamic_cast<action_dont_rotate const&>(other); }
        bool  operator==(action_dont_rotate const&  other) const { return are_equal(max_angular_accel, other.max_angular_accel, 0.0001f); }
    };

    using  collider = detail::meta::collider;
    using  collider_ptr = detail::meta::collider_ptr;

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

    modelspace  pose_frames() const { return resource().pose_frames; }
    bone_names  names() const { return resource().names; }
    bone_hierarchy  hierarchy() const { return resource().hierarchy; }
    bone_lengths  lengths() const { return resource().lengths; }
    anim_space_directions  directions() const { return resource().directions; }

    std::unordered_map<std::string, motion_template> const&  motions_map() const { return resource().motions_map; }
    motion_template const&  at(std::string const&  motion_name) const { return motions_map().at(motion_name); }
};


}

#endif
