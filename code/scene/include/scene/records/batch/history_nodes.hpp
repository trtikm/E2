#ifndef E2_SCENE_RECORDS_BATCH_HISTORY_NODES_HPP_INCLUDED
#   define E2_SCENE_RECORDS_BATCH_HISTORY_NODES_HPP_INCLUDED

#   include <scene/scene_history_nodes_default.hpp>
#   include <scene/scene_record_id.hpp>
#   include <qtgl/effects_config.hpp>
#   include <boost/filesystem/path.hpp>
#   include <string>

namespace scn {


struct  scene_history_batch_insert final : public scene_history_record_insert<scene_history_batch_insert>
{
    using super_type = scene_history_record_insert<scene_history_batch_insert>;

    scene_history_batch_insert(
            scene_record_id const&  id,
            std::string const&  batch_pathname_or_sketch_id,
            std::string const&  skin_name,
            qtgl::effects_config const  effects,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(id, as_inverse_operation)
        , m_batch_pathname_or_sketch_id(batch_pathname_or_sketch_id)
        , m_skin_name(skin_name)
        , m_effects(effects)
    {}

    std::string const&  get_batch_pathname_or_sketch_id() const { return m_batch_pathname_or_sketch_id; }
    std::string const& get_skin_name() const { return m_skin_name; }
    qtgl::effects_config  get_effects_config() const { return m_effects; }

private:
    std::string  m_batch_pathname_or_sketch_id;
    std::string  m_skin_name;
    qtgl::effects_config  m_effects;
};


struct  scene_history_batch_update_props final : public scene_history_record_update<scene_history_batch_update_props>
{
    using super_type = scene_history_record_update<scene_history_batch_update_props>;

    scene_history_batch_update_props(
            scene_record_id const&  id,
            std::string const&  old_batch_pathname_or_sketch_id,
            std::string const&  old_skin_name,
            qtgl::effects_config const  old_effects,
            std::string const&  new_batch_pathname_or_sketch_id,
            std::string const&  new_skin_name,
            qtgl::effects_config const  new_effects,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'return-to-old-props'
            )
        : super_type(id, as_inverse_operation)
        , m_old_batch_pathname_or_sketch_id(old_batch_pathname_or_sketch_id)
        , m_old_skin_name(old_skin_name)
        , m_old_effects(old_effects)
        , m_new_batch_pathname_or_sketch_id(new_batch_pathname_or_sketch_id)
        , m_new_skin_name(new_skin_name)
        , m_new_effects(new_effects)
    {}

    std::string const& get_old_batch_pathname_or_sketch_id() const { return m_old_batch_pathname_or_sketch_id; }
    std::string const& get_old_skin_name() const { return m_old_skin_name; }
    qtgl::effects_config  get_old_effects_config() const { return m_old_effects; }
    std::string const&  get_new_batch_pathname_or_sketch_id() const { return m_new_batch_pathname_or_sketch_id; }
    std::string const& get_new_skin_name() const { return m_new_skin_name; }
    qtgl::effects_config  get_new_effects_config() const { return m_new_effects; }

private:
    std::string  m_old_batch_pathname_or_sketch_id;
    std::string  m_old_skin_name;
    qtgl::effects_config  m_old_effects;
    std::string  m_new_batch_pathname_or_sketch_id;
    std::string  m_new_skin_name;
    qtgl::effects_config  m_new_effects;
};


}

#endif
