#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_TREE_WIDGET_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_TREE_WIDGET_HPP_INCLUDED

#   include <qtgl/gui_utils.hpp>
#   include <QTreeWidget>
#   include <functional>
#   include <string>
#   include <QtGui>

namespace window_tabs { namespace tab_scene {


struct tree_widget : public QTreeWidget {
private:
    Q_OBJECT
public:
    tree_widget(
            std::function<void()> const&  on_selection_changed,
            std::function<void(QTreeWidgetItem*)> const&  on_item_double_clicked
            )
        : QTreeWidget()
        , m_on_selection_changed(on_selection_changed)
        , m_on_item_double_clicked(on_item_double_clicked)
    {
        connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(on_selection_changed()));
        connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(on_item_double_clicked(QTreeWidgetItem*, int)));
    }
public slots:

    void on_selection_changed()
    {
        m_on_selection_changed();
    }

    void on_item_double_clicked(QTreeWidgetItem* item, int )
    {
        m_on_item_double_clicked(item);
    }

private:
    std::function<void()>  m_on_selection_changed;
    std::function<void(QTreeWidgetItem*)>  m_on_item_double_clicked;
};


struct  tree_widget_item : public QTreeWidgetItem
{
    explicit tree_widget_item(bool const  represents_coord_system)
        : QTreeWidgetItem()
        , m_represents_coord_system(represents_coord_system)
    {}

    bool  represents_coord_system() const { return m_represents_coord_system; }

private:

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
        tree_widget_item*  tree_item,
        std::function<void(std::string const&)> const&  names_acceptor = [](std::string const&) {}
        );


inline tree_widget_item*  find_nearest_coord_system_item(
        QTreeWidgetItem*  tree_item,
        std::function<void(std::string const&)> const&  names_acceptor = [](std::string const&) {}
        )
{
    return find_nearest_coord_system_item(as_tree_widget_item(tree_item), names_acceptor);
}


}}

#endif
