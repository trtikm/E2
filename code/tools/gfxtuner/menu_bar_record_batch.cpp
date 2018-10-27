#include <gfxtuner/menu_bar_record_batch.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_batch {


void  register_record_menu_items(std::multimap<std::string, std::pair<std::string, record_menu_item_info> >&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_batches_folder_name(),
        {
            "default",
            {
                "Insert &batch",
                "Ctrl+Alt+B",
                "A batch is an atomic block of grahical data. It can only be placed under a coordinate system. It means that at least one\n"
                "non-'@pivot' coord. system must be selected. Otherwise the operation will fail (with an error message to the status bar).\n"
                "For each selected non-'@pivot' coord. system there will be created a copy of the newly created batch under that system.\n"
                "Batches are available on the disc under models root directory 'E2/dist/data/shared/gfx/models'. Finally, the newly created\n"
                "batch will become the only selected object in the scene."
            }
        }
    });
}


}}
