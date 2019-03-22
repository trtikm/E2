"""
The script selects all vertices of a selected single object which are attached to NO bone. The script can
thus be useful for finalising a rigged object before its export via E2 export plug-in (because the plug-in
will refuse to export object with a vertex not attached to any bone).
"""

import bpy

print("*************************************************************************************")
bpy.ops.object.mode_set(mode='OBJECT')
vertices = bpy.context.active_object.data.vertices
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_mode(type="VERT")
bpy.ops.mesh.select_all(action='DESELECT')
bpy.ops.object.mode_set(mode='OBJECT')

for idx in range(len(vertices)):
    if len(vertices[idx].groups) == 0:
        vertices[idx].select = True

bpy.ops.object.mode_set(mode='EDIT')
