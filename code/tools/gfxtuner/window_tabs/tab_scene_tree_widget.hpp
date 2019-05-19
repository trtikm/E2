#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_TREE_WIDGET_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_TREE_WIDGET_HPP_INCLUDED

#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <qtgl/gui_utils.hpp>
#   include <QTreeWidget>
#   include <QIcon>
#   include <functional>
#   include <string>
#   include <unordered_map>
#   include <QtGui>

namespace window_tabs { namespace tab_scene {


struct tree_widget;


struct  tree_widget_item : public QTreeWidgetItem
{
    explicit tree_widget_item(bool const  represents_coord_system)
        : QTreeWidgetItem()
        , m_represents_coord_system(represents_coord_system)
    {}

    bool  represents_coord_system() const { return m_represents_coord_system; }

private:
    friend struct tree_widget;

    using QTreeWidgetItem::addChild;
    using QTreeWidgetItem::takeChild;

    bool  m_represents_coord_system;
};


inline tree_widget_item*  as_tree_widget_item(QTreeWidgetItem* const tree_item)
{
    return  dynamic_cast<tree_widget_item*>(tree_item);
}


inline tree_widget_item const*  as_tree_widget_item(QTreeWidgetItem const* const tree_item)
{
    return  dynamic_cast<tree_widget_item const*>(tree_item);
}


struct tree_widget : public QTreeWidget {
private:
    Q_OBJECT
public:
    tree_widget(
            std::function<void()> const&  on_selection_changed,
            std::function<void(tree_widget_item*)> const&  on_item_double_clicked_,
            std::function<void()> const&  on_escape_widget_
            )
        : QTreeWidget()
        , m_on_selection_changed(on_selection_changed)
        , m_on_item_double_clicked(on_item_double_clicked_)
        , m_on_escape_widget(on_escape_widget_)
        , m_from_nodes_to_widgets()
        , m_from_widgets_to_nodes()
    {
        connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(on_selection_changed()));
        connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(on_item_double_clicked(QTreeWidgetItem*, int)));
    }

    tree_widget_item*  insert(std::string const&  text, QIcon const&  icon, bool const  represents_coord_system, tree_widget_item* const  parent_tree_item);
    void  erase(tree_widget_item* const  item_ptr);

    void  clear();

    tree_widget_item*  find(scn::scene_record_id const&  id) const;
    tree_widget_item*  find(scn::scene_node_id const&  id, std::string const&  folder_name) const { return find(scn::scene_record_id{ id, folder_name }); }
    tree_widget_item*  find(scn::scene_node_id const&  id) const { return find(id, ""); }

public slots:

    void on_selection_changed()
    {
        m_on_selection_changed();
    }

    void on_item_double_clicked(QTreeWidgetItem* item, int )
    {
        m_on_item_double_clicked(as_tree_widget_item(item));
    }

private:
    using QTreeWidget::clear;
    using QTreeWidget::addTopLevelItem;
    using QTreeWidget::takeTopLevelItem;

    void  keyReleaseEvent(QKeyEvent* const event) override;

    std::function<void()>  m_on_selection_changed;
    std::function<void(tree_widget_item*)>  m_on_item_double_clicked;
    std::function<void()>  m_on_escape_widget;

    using  from_nodes_to_widgets_map = std::unordered_map<scn::scene_record_id, tree_widget_item*>;
    using  from_widgets_to_nodes_map = std::unordered_map<tree_widget_item*, from_nodes_to_widgets_map::iterator>;
    from_nodes_to_widgets_map  m_from_nodes_to_widgets;
    from_widgets_to_nodes_map  m_from_widgets_to_nodes;
};


inline std::string  get_tree_widget_item_name(QTreeWidgetItem const* const  tree_item)
{
    return qtgl::to_string(tree_item->text(0));
}


inline bool  represents_coord_system(tree_widget_item const* const  item)
{
    return item != nullptr && item->represents_coord_system();
}


inline bool  represents_coord_system(QTreeWidgetItem const* const  item)
{
    return represents_coord_system(as_tree_widget_item(item));
}


inline bool  represents_folder(tree_widget_item const* const  item)
{
    return item != nullptr && !represents_coord_system(item) && represents_coord_system(item->parent());
}


inline bool  represents_folder(QTreeWidgetItem const* const  item)
{
    return represents_folder(as_tree_widget_item(item));
}


inline bool  represents_record(tree_widget_item const* const  item)
{
    return item != nullptr && represents_folder(item->parent()) && item->childCount() == 0;
}


inline bool  represents_record(QTreeWidgetItem const* const  item)
{
    return represents_record(as_tree_widget_item(item));
}


tree_widget_item*  find_nearest_coord_system_item(
        tree_widget_item const*  tree_item,
        std::function<void(std::string const&)> const&  names_acceptor = [](std::string const&) {}
        );


inline tree_widget_item*  find_nearest_coord_system_item(
        QTreeWidgetItem const*  tree_item,
        std::function<void(std::string const&)> const&  names_acceptor = [](std::string const&) {}
        )
{
    return find_nearest_coord_system_item(as_tree_widget_item(tree_item), names_acceptor);
}


}}

#endif
