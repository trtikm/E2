#include <gfxtuner/menu_bar_record_batch.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_batch {


void  register_record_menu_items(std::vector<record_menu_item_info>&  record_menu_items)
{
    record_menu_items.push_back({
        "Insert &batch",
        "Ctrl+Alt+B",
        "A batch is an atomic block of grahical data. It can only be placed under a coordinate system. It means that at least one\n"
        "non-'@pivot' coord. system must be selected. Otherwise the operation will fail (with an error message to the status bar).\n"
        "For each selected non-'@pivot' coord. system there will be created a copy of the newly created batch under that system.\n"
        "Batches are available on the disk under meshes root directory '<E2-root-dir>/dist/data/shared/gfx/meshes'. The newly created\n"
        "batch will become the only selected object in the scene.\n"
        "Use 'Blender' and 'E2 Blender export plugin' (located in '<E2-root-dir>/data/Blender/export/E2_Blender_gfx_export_plugin.py')\n"
        "to create new batches.",
        scn::get_batches_folder_name(),
        "default"   // mode
        });
}


}}
