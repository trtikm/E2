import os
import bpy


E2_export_dir = os.path.join(os.getcwd(), "E2_export")
precision_level = 0.25
depth = 0.1


def create_character(c, precision_level=0.25, depth=0.1):
    bpy.ops.object.text_add()
    bpy.ops.object.editmode_toggle()
    bpy.ops.font.select_all()
    bpy.ops.font.delete(type='PREVIOUS_OR_SELECTION')
    bpy.ops.font.text_insert(text=c)
    bpy.ops.object.editmode_toggle()
    bpy.context.object.data.extrude = 0.1 * depth
    bpy.context.object.data.resolution_u = int(20.0 * precision_level)
    bpy.ops.object.convert(target='MESH')
    bpy.context.object.name = "font/" + str(ord(c))
    bpy.context.object.data.name = "font/" + str(ord(c))
    bpy.ops.object.editmode_toggle()
    bpy.ops.mesh.select_all(action='TOGGLE')
    bpy.ops.object.editmode_toggle()
    bpy.ops.transform.resize(value=(1.45, 1.45, 1.45))
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.context.object.data.name = "font/" + str(ord(c))


def export_font():
    if not os.path.isdir(E2_export_dir):
        os.makedirs(E2_export_dir)
    for i in range(33, 127):
        create_character(chr(i), precision_level, depth)
        bpy.ops.export.e2_gfx_exporter(directory=E2_export_dir)
        bpy.ops.object.delete(use_global=False)


export_font()
# create_character("W", precision_level, depth)
