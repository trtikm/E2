#include <gfxtuner/menu_bar_record_rigid_body.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_rigid_body {


void  register_record_menu_items(std::multimap<std::string, std::pair<std::string, record_menu_item_info> >&  record_menu_items)
{
    record_menu_items.insert({
        scn::get_rigid_body_folder_name(),
        {
            "default",
            {
                "Insert r&igid body",
                "Ctrl+Alt+R",
                "A 'rigid body' is non-deformable object in 3D space whose motion in the space\n"
                "approximatelly agrees with physics laws of rigid body dynamics. A rigid body has\n"
                "linear and angular momentum, and forces and torques may acting on it. When inserted\n"
                "to the scene under a certain 'coordinate system' node, then the simulated motion\n"
                "of the rigid body will update the position and rotation of that coordinate system.\n"
                "The shape, mass, and inertia tensor of the rigid body are given by 'collider' instances\n"
                "anywhere in the sub-tree of the mentioned coordinate system node.\n"
                "NOTE: The origin of the coordinate system of the rigid body is assumed to be at the\n"
                "      center of mass of the rigid body. The origin is thus automatically moved to the\n"
                "      center of mass of the rigid body during the simulation. Therefore, you may see\n"
                "      in the first simulation step the coordinate system 'jumps' to the center of mass."
            }
        }
    });
}


}}
