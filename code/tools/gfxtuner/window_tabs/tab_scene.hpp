#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <QWidget>
#   include <QTreeWidget>
#   include <QColor>


struct  program_window;


namespace window_tabs { namespace tab_scene {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    QTreeWidget*  scene_tree() const { return m_scene_tree; }

    void  on_scene_hierarchy_item_selected(QTreeWidgetItem* const tree_item, int const column);
    void  on_scene_insert_coord_system();
    void  on_scene_insert_batch();
    void  on_scene_erase_selected();

    void  save();

private:
    program_window*  m_wnd;

    QTreeWidget*  m_scene_tree;
};


QWidget*  make_scene_tab_content(widgets const&  w);


}}

#endif
