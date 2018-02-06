#include <gfxtuner/scene_history.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>


namespace { namespace detail {


struct  scene_history_commit final : public scene_history_node
{
    scene_history_commit()
        : m_id(++s_id_counter)
    {}

    void  undo() const override { UNREACHABLE(); }
    void  redo() const override { UNREACHABLE(); }

    natural_64_bit  id() const { return m_id; }

    bool  is_mutator() const override { return false; }

private:
    natural_64_bit  m_id;
    static natural_64_bit  s_id_counter;
};

natural_64_bit  scene_history_commit::s_id_counter = 0ULL;

inline bool  is_commit_node(scene_history_node_ptr const  node_ptr)
{
    return std::dynamic_pointer_cast<scene_history_commit const>(node_ptr) != nullptr;
}

inline natural_64_bit  get_commit_node_id(scene_history_node_ptr const  node_ptr)
{
    return std::dynamic_pointer_cast<scene_history_commit const>(node_ptr)->id();
}


}}


scene_history&  scene_history::get_instance()
{
    static scene_history  history;
    return history;
}


scene_history::scene_history()
    : m_history({ scene_history_node_ptr(new detail::scene_history_commit) })
    , m_active_commit(0UL)
{}


void  scene_history::insert(scene_history_node_ptr const  node_ptr)
{
    ASSUMPTION(node_ptr != nullptr);
    ASSUMPTION(m_active_commit < m_history.size());
    ASSUMPTION(detail::is_commit_node(m_history.at(m_active_commit)));
    if (m_active_commit + 1UL < m_history.size() && detail::is_commit_node(m_history.at(m_history.size() - 1UL)))
        m_history.resize(m_active_commit + 1UL);
    m_history.push_back(node_ptr);
}

void  scene_history::commit()
{
    ASSUMPTION(m_active_commit < m_history.size() - 1UL);
    ASSUMPTION(!detail::is_commit_node(m_history.at(m_history.size() - 1UL)));
    insert<detail::scene_history_commit>();
    m_active_commit = m_history.size() - 1UL;
    INVARIANT(detail::is_commit_node(m_history.at(m_active_commit)));
}

void  scene_history::undo()
{
    ASSUMPTION(m_active_commit < m_history.size());
    ASSUMPTION(detail::is_commit_node(m_history.at(m_active_commit)));
    if (m_active_commit > 0UL)
        for (--m_active_commit; !detail::is_commit_node(m_history.at(m_active_commit)); --m_active_commit)
            m_history.at(m_active_commit)->undo();
    INVARIANT(detail::is_commit_node(m_history.at(m_active_commit)));
}

void  scene_history::redo()
{
    ASSUMPTION(m_active_commit < m_history.size());
    ASSUMPTION(detail::is_commit_node(m_history.at(m_active_commit)));
    if (m_active_commit < m_history.size() - 1UL)
        for (++m_active_commit; detail::is_commit_node(m_history.at(m_active_commit)); ++m_active_commit)
            m_history.at(m_active_commit)->redo();
    INVARIANT(detail::is_commit_node(m_history.at(m_active_commit)));
}

void  scene_history::clear()
{
    m_history.resize(1UL);
    m_active_commit = 0UL;
}

natural_64_bit  scene_history::get_active_commit_id() const
{
    ASSUMPTION(m_active_commit < m_history.size());
    ASSUMPTION(detail::is_commit_node(m_history.at(m_active_commit)));
    return detail::get_commit_node_id(m_history.at(m_active_commit));
}

bool  scene_history::was_applied_mutator_since_commit(natural_64_bit const  commit_id) const
{
    if (m_active_commit > 0UL)
        for (std::size_t  i = m_active_commit - 1UL; i != 0UL; --i)
            if (detail::is_commit_node(m_history.at(i)))
            {
                if (detail::get_commit_node_id(m_history.at(i)) == commit_id)
                    break;
            }
            else if (m_history.at(i)->is_mutator())
                return true;
    if (m_active_commit < m_history.size() - 1UL)
        for (std::size_t i = m_active_commit + 1UL; i != m_history.size(); ++i)
            if (detail::is_commit_node(m_history.at(i)))
            {
                if (detail::get_commit_node_id(m_history.at(i)) == commit_id)
                    break;
            }
            else if (m_history.at(i)->is_mutator())
                return true;
    return false;
}
