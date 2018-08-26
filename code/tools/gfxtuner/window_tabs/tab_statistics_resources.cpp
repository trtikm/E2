#include <gfxtuner/window_tabs/tab_statistics_resources.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/algorithm/string.hpp>
#include <QVBoxLayout>
#include <QString>
#include <vector>
#include <memory>

namespace {


std::string  get_tree_resource_uid(async::key_type const&  key)
{
    if (boost::starts_with(key.get_unique_id(), async::key_type::get_prefix_of_generated_fresh_unique_id()))
        return async::key_type::get_prefix_of_generated_fresh_unique_id();
    return key.get_unique_id();
}


}

namespace window_tabs { namespace tab_statistics { namespace tab_resources {


widgets::widgets(program_window* const  wnd)
    : QTreeWidget()
    , m_wnd(wnd)
    , m_icon_data_type((boost::filesystem::path{ get_program_options()->dataRoot() } /
                       "shared/gfx/icons/data_type.png").string().c_str())
    , m_icon_wainting_for_load((boost::filesystem::path{ get_program_options()->dataRoot() } /
                               "shared/gfx/icons/wait.png").string().c_str())
    , m_icon_being_loaded((boost::filesystem::path{ get_program_options()->dataRoot() } /
                          "shared/gfx/icons/loading.png").string().c_str())
    , m_icon_in_use((boost::filesystem::path{ get_program_options()->dataRoot() } /
                    "shared/gfx/icons/in_use.png").string().c_str())
    , m_icon_failed_to_load((boost::filesystem::path{ get_program_options()->dataRoot() } /
                            "shared/gfx/icons/error.png").string().c_str())
    , m_records()
    , m_just_being_loaded(async::get_invalid_key())
{
    setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

    {
        QTreeWidgetItem* const  headerItem = new QTreeWidgetItem();
        headerItem->setText(0, QString("Resource types"));
        headerItem->setText(1, QString("#Refs"));
        setHeaderItem(headerItem);
    }
}


void  widgets::on_time_event()
{
    TMPROF_BLOCK();

    async::statistics_of_cached_resources  new_records;
    async::key_type const  new_just_being_loaded = async::get_statistics_of_cached_resources(new_records);

    // Build a list of changes in the cache since the previous call.

    using update_record_type = std::pair<async::statistics_of_cached_resources::value_type const*,  // Old record ptr
                                         async::statistics_of_cached_resources::value_type const*>; // New record ptr
    std::vector<update_record_type> update_records;
    for (auto const&  new_elem : new_records)
    {
        auto const  rec_it = m_records.find(new_elem.first);
        if (rec_it == m_records.cend())
            update_records.push_back({nullptr, &new_elem});
        else if (rec_it->second != new_elem.second)
            update_records.push_back({ &*rec_it, &new_elem });
    }
    for (auto const& elem : m_records)
    {
        auto const  rec_it = new_records.find(elem.first);
        if (rec_it == new_records.cend())
            update_records.push_back({ &elem, nullptr });
    }
    if (m_just_being_loaded == async::get_invalid_key() && new_just_being_loaded != async::get_invalid_key())
    {
        auto const  new_rec_it = m_records.find(new_just_being_loaded);
        if (new_rec_it != m_records.cend())
            update_records.push_back({ &*new_rec_it, &*new_records.find(new_just_being_loaded) });
    }

    // Write the changes to the QTreeWidget.

    for (auto const&  update_record : update_records)
        if (update_record.first == nullptr)
            insert_record_to_the_tree(
                    update_record.second->first,
                    update_record.second->second,
                    update_record.second->first == new_just_being_loaded
                    );
        else if (update_record.second == nullptr)
            erase_record_from_the_tree(update_record.first->first, update_record.first->second);
        else
            update_record_in_the_tree(
                    update_record.second->first,
                    update_record.first->second,
                    update_record.second->second,
                    update_record.second->first == new_just_being_loaded
                    );
    if (!update_records.empty())
        sortItems(0, Qt::SortOrder::AscendingOrder);

    // And finaly save the new values for the use in the next update.

    m_records.swap(new_records);
    m_just_being_loaded = new_just_being_loaded;
}


void  widgets::insert_record_to_the_tree(
        async::key_type const&  key,
        async::cached_resource_info const&  info,
        bool const  is_just_being_loaded
        )
{
    TMPROF_BLOCK();

    QTreeWidgetItem*  data_type_node = nullptr;
    {
        auto const  items = findItems(QString(key.get_data_type_name().c_str()), Qt::MatchFlag::MatchExactly, 0);
        if (items.empty())
        {
            std::unique_ptr<QTreeWidgetItem>  tree_node(new QTreeWidgetItem);
            tree_node->setText(0, QString(key.get_data_type_name().c_str()));
            tree_node->setIcon(0, m_icon_data_type);
            tree_node->setText(1, QString("0"));
            addTopLevelItem(tree_node.get());
            data_type_node = tree_node.release();
        }
        else
        {
            INVARIANT(items.size() == 1);
            data_type_node = items.front();
        }
    }
    QTreeWidgetItem*  uid_node = nullptr;
    {
        std::string const  uid = get_tree_resource_uid(key);
        for (int i = 0, n = data_type_node->childCount(); i != n; ++i)
        {
            auto const  item = data_type_node->child(i);
            std::string const  item_name = qtgl::to_string(item->text(0));
            if (item_name == uid)
            {
                uid_node = item;
                break;
            }
        }
    }
    if (uid_node == nullptr)
    {
        std::unique_ptr<QTreeWidgetItem>  tree_node(new QTreeWidgetItem);
        tree_node->setText(0, QString(get_tree_resource_uid(key).c_str()));
        tree_node->setText(1, QString(std::to_string(info.get_ref_count()).c_str()));
        data_type_node->addChild(tree_node.get());
        uid_node = tree_node.release();
    }
    else
    {
        std::string const  item_refs_name = qtgl::to_string(uid_node->text(1));
        int const  old_num_refs = std::atoi(item_refs_name.c_str());
        uid_node->setText(1, QString(std::to_string(old_num_refs + info.get_ref_count()).c_str()));
    }
    uid_node->setIcon(0, *choose_icon(info.get_load_state(), is_just_being_loaded));
    if (info.get_error_message().empty())
        uid_node->setToolTip(0, "");
    else
        uid_node->setToolTip(0, QString(info.get_error_message().c_str()));

    std::string const  item_refs_name = qtgl::to_string(data_type_node->text(1));
    int const  old_num_refs = std::atoi(item_refs_name.c_str());
    data_type_node->setText(1, QString(std::to_string(old_num_refs + info.get_ref_count()).c_str()));
}


void  widgets::erase_record_from_the_tree(
        async::key_type const&  key,
        async::cached_resource_info const&  info
        )
{
    TMPROF_BLOCK();

    QTreeWidgetItem*  data_type_node = nullptr;
    {
        auto const  items = findItems(QString(key.get_data_type_name().c_str()), Qt::MatchFlag::MatchExactly, 0);
        if (items.empty())
            return;
        INVARIANT(items.size() == 1);
        data_type_node = items.front();
    }
    QTreeWidgetItem*  uid_node = nullptr;
    {
        std::string const  uid = get_tree_resource_uid(key);
        for (int i = 0, n = data_type_node->childCount(); i != n; ++i)
        {
            auto const  item = data_type_node->child(i);
            std::string const  item_name = qtgl::to_string(item->text(0));
            if (item_name == uid)
            {
                uid_node = item;
                break;
            }
        }
    }
    if (uid_node != nullptr)
    {
        if (qtgl::to_string(uid_node->text(0)) != key.get_unique_id())
        {
            std::string const  item_refs_name = qtgl::to_string(uid_node->text(1));
            int const  old_num_refs = std::atoi(item_refs_name.c_str());
            if (old_num_refs - info.get_ref_count() > 0)
                uid_node->setText(1, QString(std::to_string(old_num_refs - info.get_ref_count()).c_str()));
            else
                delete data_type_node->takeChild(data_type_node->indexOfChild(uid_node));
        }
        else
            delete data_type_node->takeChild(data_type_node->indexOfChild(uid_node));
    }
    if (data_type_node->childCount() == 0)
        delete takeTopLevelItem(indexOfTopLevelItem(data_type_node));
    else
    {
        std::string const  item_refs_name = qtgl::to_string(data_type_node->text(1));
        int const  old_num_refs = std::atoi(item_refs_name.c_str());
        data_type_node->setText(1, QString(std::to_string(old_num_refs - info.get_ref_count()).c_str()));
    }
}


void  widgets::update_record_in_the_tree(
        async::key_type const&  key,
        async::cached_resource_info const&  old_info,
        async::cached_resource_info const&  new_info,
        bool const  is_just_being_loaded
        )
{
    TMPROF_BLOCK();

    QTreeWidgetItem*  data_type_node = nullptr;
    {
        auto const  items = findItems(QString(key.get_data_type_name().c_str()), Qt::MatchFlag::MatchExactly, 0);
        INVARIANT(items.size() == 1);
        data_type_node = items.front();
    }
    QTreeWidgetItem*  uid_node = nullptr;
    {
        std::string const  uid = get_tree_resource_uid(key);
        for (int i = 0, n = data_type_node->childCount(); i != n; ++i)
        {
            auto const  item = data_type_node->child(i);
            std::string const  item_name = qtgl::to_string(item->text(0));
            if (item_name == uid)
            {
                uid_node = item;
                break;
            }
        }
        INVARIANT(uid_node != nullptr);
    }

    uid_node->setIcon(0, *choose_icon(new_info.get_load_state(), is_just_being_loaded));
    if (qtgl::to_string(uid_node->text(0)) != key.get_unique_id())
    {
        std::string const  item_refs_name = qtgl::to_string(uid_node->text(1));
        int const  old_num_refs = std::atoi(item_refs_name.c_str());
        int const  new_num_refs = old_num_refs + ((int)new_info.get_ref_count() - (int)old_info.get_ref_count());
        uid_node->setText(1, QString(std::to_string(new_num_refs).c_str()));
    }
    else
    {
        uid_node->setText(1, QString(std::to_string(new_info.get_ref_count()).c_str()));
    }
    if (new_info.get_error_message().empty())
        uid_node->setToolTip(0, "");
    else
        uid_node->setToolTip(0, QString(new_info.get_error_message().c_str()));

    std::string const  item_refs_name = qtgl::to_string(data_type_node->text(1));
    int const  old_num_refs = std::atoi(item_refs_name.c_str());
    int const  new_num_refs = old_num_refs + ((int)new_info.get_ref_count() - (int)old_info.get_ref_count());
    data_type_node->setText(1, QString(std::to_string(new_num_refs).c_str()));
}


QIcon const*  widgets::choose_icon(async::LOAD_STATE const  load_state, bool const  is_just_being_loaded) const
{
    TMPROF_BLOCK();

    if (is_just_being_loaded)
    {
        ASSUMPTION(load_state == async::LOAD_STATE::IN_PROGRESS);
        return &m_icon_being_loaded;
    }
    switch (load_state)
    {
    case async::LOAD_STATE::IN_PROGRESS: return &m_icon_wainting_for_load;
    case async::LOAD_STATE::FINISHED_SUCCESSFULLY: return &m_icon_in_use;
    case async::LOAD_STATE::FINISHED_WITH_ERROR: return &m_icon_failed_to_load;
    default: UNREACHABLE();
    }
}


}}}
