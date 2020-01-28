#include <gfxtuner/menu_bar_record_collider.hpp>
#include <scene/scene_utils_specialised.hpp>

namespace record_menu_items { namespace record_collider {


void  register_record_menu_items(std::vector<record_menu_item_info>&  record_menu_items)
{
    record_menu_items.push_back({
        "Insert collision c&apsule",
        "Ctrl+Alt+C",
        "A 'capsule' is a collision object, i.e. a 'collider', whose shape consists of a torus with\n"
        "a hemisphere on the top and bottom of the cilinder. The shape of a capsule can be fully described\n"
        "by two numbers: 1. 'Excentricity' which is the distance of the hemispheres (or equally the length\n"
        "of the axis of the cylinder), and 2. 'Radius' which is the common radius of both hemispheres\n"
        "(or equally as the thickness of the cyinder from its axis).\n"
        "Each collider is further assigned a 'material', like WOOD or STEEL, which is used in computation of\n"
        "density, mass, inertia tensor, and friction coefficients. Finally, a collider can be maked as either\n"
        "'static' or 'dynamic'. A static collider cannot be moved (think of it as a part of ground). A dynamic\n"
        "collider is free to move in space and time, i.e. it can be moved together with an associated 'rigid body'.",
        scn::get_collider_folder_name(),
        "capsule",
        });
    record_menu_items.push_back({
        "Insert collision sp&here",
        "Ctrl+Alt+S",
        "A 'sphere' is a collision object, i.e. a 'collider', whose shape can be fully described\n"
        "by one number: 'Radius' of the sphere.\n"
        "Each collider is further assigned a 'material', like WOOD or STEEL, which is used in computation of\n"
        "density, mass, inertia tensor, and friction coefficients. Finally, a collider can be maked as either\n"
        "'static' or 'dynamic'. A static collider cannot be moved (think of it as a part of ground). A dynamic\n"
        "collider is free to move in space and time, i.e. it can be moved together with an associated 'rigid body'.",
        scn::get_collider_folder_name(),
        "sphere",
        });
    record_menu_items.push_back({
        "Insert collision triangle &mesh",
        "Ctrl+Alt+M",
        "A 'triangle mesh' is simply a collection of triangles. A 'triangle' is then a collision object,\n"
        "i.e. a 'collider', whose shape can be fully described by 3 points in 3D space. Triangles in a triangle\n"
        "mesh are defined in two files 'vertices.txt' and 'indices.txt' representing the vertex buffer\n"
        "and index buffer of the triangles respectively. Several of these files are already available for use\n"
        "under meshes root directory '<E2-root-dir>/dist/data/shared/gfx/meshes/'. You can also use 'Blender' and\n"
        "'E2 Blender export plugin' (located in '<E2-root-dir>/data/Blender/export/E2_Blender_gfx_export_plugin.py')\n"
        "to create new files.\n"
        "Each collider is further assigned a 'material', like WOOD or STEEL, which is used in computation of\n"
        "density, mass, inertia tensor, and friction coefficients. Finally, a collider can be maked as either\n"
        "'static' or 'dynamic'. A static collider cannot be moved (think of it as a part of ground). A dynamic\n"
        "collider is free to move in space and time, i.e. it can be moved together with an associated 'rigid body'.",
        scn::get_collider_folder_name(),
        "triangle mesh",
        });
}


}}
