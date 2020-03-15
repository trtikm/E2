#include <ai/sensor.hpp>
#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const&  default_sensor_configs()
{
    static natural_32_bit  edit_order = 0U;
    static std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const  cfg {
        { SENSOR_KIND::TOUCH_BEGIN, {
                } },
        { SENSOR_KIND::TOUCHING, {
                } },
        { SENSOR_KIND::TOUCH_END, {
                } },
        { SENSOR_KIND::TIMER, {
                { "period_in_seconds", { property_map::make_float(1.0f), true,
                        "A time period for repetitive sending an event to the owner.\n"
                        "If the period is <= 0.0001f, then the event is sent every time step.",
                        edit_order++} },
                { "consumed_in_seconds", { property_map::make_float(0.0f), true,
                        "Time already consumed from the current period.",
                        edit_order++} }
                } }
    };
    return cfg;
}


sensor::collision_contact_record::collision_contact_record(
        scene::node_id const&  collider_nid_,
        scene::collicion_contant_info_ptr const  contact_info_,
        object_id const&  other_id_,
        scene::node_id const&  other_collider_nid_
        )
    : collider_nid(collider_nid_)
    , contact_info(contact_info_)
    , other_id(other_id_)
    , other_collider_nid(other_collider_nid_)
{}


sensor::sensor(simulator* const  simulator_,
               SENSOR_KIND const  kind_,
               object_id const&  self_id_,
               scene::record_id const&  self_rid_,
               object_id const&  owner_id_,
               bool const  enabled_,
               std::shared_ptr<property_map> const  cfg_,
               std::shared_ptr<std::vector<scene::node_id> > const  collider_nids_
               )
    : m_simulator(simulator_)
    , m_kind(kind_)
    , m_self_id(self_id_)
    , m_self_rid(self_rid_)
    , m_owner_id(owner_id_)
    , m_enabled(enabled_)

    , m_cfg(cfg_)

    , m_collider_nids(collider_nids_)
    , m_collision_contacts_buffer()
    , m_touch_begin_ids()
    , m_touching_ids()
    , m_old_touching_ids()
    , m_touch_end_ids()
{
    ASSUMPTION(
        m_simulator != nullptr &&
        m_self_id.valid() && m_self_id.kind == OBJECT_KIND::SENSOR &&
        [](SENSOR_KIND const  kind, property_map const&  cfg) -> bool {
            auto const&  default_cfg = default_sensor_configs().at(kind);
            if (cfg.size() != default_cfg.size())
                return false;
            for (auto const&  elem : default_cfg)
            {
                auto const  it = cfg.find(elem.first);
                if (it == cfg.end())
                {
                    if (elem.second.is_mandatory)
                        return false;
                }
                else if (it->second->get_type() != elem.second.value->get_type())
                    return false;
            }
            return true;
            }(m_kind, *m_cfg) &&
        m_collider_nids != nullptr
        );

    for (scene::node_id const&  nid : *m_collider_nids)
    {
        ASSUMPTION(m_self_rid.get_node_id().is_prefix_of(nid));
        m_simulator->get_scene_ptr()->register_to_collision_contacts_stream(nid, m_self_id);
    }
}


sensor::~sensor()
{
    for (scene::node_id const&  nid : *m_collider_nids)
        m_simulator->get_scene_ptr()->unregister_from_collision_contacts_stream(nid, m_self_id);
}


void  sensor::set_owner_id(object_id const&  id)
{
    ASSUMPTION(id.valid() && id.kind != OBJECT_KIND::SENSOR);
    m_owner_id = id;
}


void  sensor::next_round(float_32_bit const  time_step_in_seconds)
{
    if (!is_enabled())
        return;

    if (!m_owner_id.valid())
        return;

    if (get_kind() == SENSOR_KIND::TIMER)
    {
        float_32_bit const  period = m_cfg->get_float("period_in_seconds");
        if (period <= 0.0001f)
            m_simulator->on_sensor_event(*this);
        else
        {
            float_32_bit&  consumed = m_cfg->get_float_ref("consumed_in_seconds");
            consumed += time_step_in_seconds;
            while (consumed >= period)
            {
                consumed -= period;
                m_simulator->on_sensor_event(*this);
            }
        }
    }
    else
    {
        for (collision_contact_record const& record : m_collision_contacts_buffer)
            m_touching_ids.insert(record.other_id);

        auto const  get_other_sensor_ptr = [this](object_id const&  other_oid) -> sensor const* {
            return other_oid.valid() ? &m_simulator->get_sensors().at(other_oid.index) : nullptr;
        };

        if (get_kind() == SENSOR_KIND::TOUCH_BEGIN)
        {
            for (object_id  id : m_touching_ids)
                if (m_old_touching_ids.count(id) == 0UL)
                    m_touch_begin_ids.insert(id);
            if (!m_touch_begin_ids.empty())
            {
                for (object_id const& other_oid : m_touch_begin_ids)
                    m_simulator->on_sensor_event(*this, get_other_sensor_ptr(other_oid));
            }
            m_touch_begin_ids.clear();
        }
        else if (get_kind() == SENSOR_KIND::TOUCH_END)
        {
            for (object_id  id : m_old_touching_ids)
                if (m_touching_ids.count(id) == 0UL)
                    m_touch_end_ids.insert(id);
            if (!m_touch_end_ids.empty())
            {
                for (object_id const& other_oid : m_touch_end_ids)
                    m_simulator->on_sensor_event(*this, get_other_sensor_ptr(other_oid));
            }
            m_touch_end_ids.clear();
        }
        else
        {
            INVARIANT(get_kind() == SENSOR_KIND::TOUCHING);
            for (object_id const&  other_oid : m_touching_ids)
                m_simulator->on_sensor_event(*this, get_other_sensor_ptr(other_oid));
        }

        m_touching_ids.swap(m_old_touching_ids);
        m_touching_ids.clear();
    }

    m_collision_contacts_buffer.clear();
}


void  sensor::on_collision_contact(
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info_ptr const  contact_info,
        object_id const&  other_id,
        scene::node_id const&  other_collider_nid
        )
{
    if (!is_enabled())
        return;

    if (!m_owner_id.valid())
        return;

    INVARIANT(!other_id.valid() || other_id.kind == OBJECT_KIND::SENSOR);

    m_collision_contacts_buffer.push_back({
            collider_nid,
            contact_info,
            other_id,
            other_collider_nid
            });
}


}
