#include <gfxtuner/scene_history.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>


namespace { namespace detail {


struct  scene_history_commit final : public scene_history_node
{
    void  undo() const override { UNREACHABLE(); }
    void  redo() const override { UNREACHABLE(); }
};

inline bool  is_commit_node(scene_history_node_ptr const  node_ptr)
{
    return std::dynamic_pointer_cast<scene_history_commit const>(node_ptr) != nullptr;
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
