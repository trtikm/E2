import bpy
import os

def test_loop_vertices(mesh,fname):
    os.makedirs(os.path.dirname(fname), exist_ok=True)
    f = open(fname,"w")

    f.write("len(mesh.vertices) = " + str(len(mesh.vertices)) + "\n")
    f.write("len(mesh.edges) = " + str(len(mesh.edges)) + "\n")
    f.write("len(mesh.polygons) = " + str(len(mesh.polygons)) + "\n")
    f.write("len(mesh.loops) = " + str(len(mesh.loops)) + "\n")
    f.write("len(mesh.uv_layers) = " + str(len(mesh.uv_layers)) + "\n")
    f.write("len(mesh.vertex_colors) = " + str(len(mesh.vertex_colors)) + "\n")
    f.write("len(mesh.materials) = " + str(len(mesh.materials)) + "\n\n")

    f.write("mesh.vertices[*].co[0/1/2] = [")
    brk = 3
    for i in range(0,len(mesh.vertices)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        v = mesh.vertices[i].co
        f.write("(" + ("%.3f"%v[0]) + ", " + ("%.3f"%v[1]) + ", " + ("%.3f"%v[2]) + "),  ")
    f.write("]\n\n")

    f.write("mesh.vertices[*].normal[0/1/2] = [")
    brk = 3
    for i in range(0,len(mesh.vertices)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        v = mesh.vertices[i].normal
        f.write("(" + ("%.3f"%v[0]) + ", " + ("%.3f"%v[1]) + ", " + ("%.3f"%v[2]) + "),  ")
    f.write("]\n\n")
        
    f.write("mesh.vertices[*].index = [")
    brk = 10
    for i in range(0,len(mesh.vertices)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.vertices[i].index) + ", ")
    f.write("]\n\n")

    f.write("mesh.edges[*].vertices[0/1] = [")
    brk = 5
    for i in range(0,len(mesh.edges)):
        if brk == 5:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        v = mesh.edges[i].vertices
        f.write("(" + str(v[0]) + ", " + str(v[1]) + "),  ")
    f.write("]\n\n")

    f.write("mesh.edges[*].index = [")
    brk = 10
    for i in range(0,len(mesh.edges)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.edges[i].index) + ", ")
    f.write("]\n\n")

    f.write("len(mesh.polygons[*].vertices) = [")
    brk = 10
    for i in range(0,len(mesh.polygons)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(len(mesh.polygons[i].vertices)) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.polygons[*].vertices[*] = [ ")
    brk = 3
    for i in range(0,len(mesh.polygons)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write("[")
        for v in mesh.polygons[i].vertices:
            f.write(str(v) + ", ")
        f.write("],  ")
    f.write("]\n\n")

    f.write("len(mesh.polygons[*].loop_indices) = [")
    brk = 10
    for i in range(0,len(mesh.polygons)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(len(mesh.polygons[i].loop_indices)) + ", ")
    f.write("]\n\n")

    f.write("mesh.polygons[*].loop_indices[*] = [ ")
    brk = 3
    for i in range(0,len(mesh.polygons)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write("[")
        for v in mesh.polygons[i].loop_indices:
            f.write(str(v) + ", ")
        f.write("],  ")
    f.write("]\n\n")

    f.write("mesh.polygons[*].loop_start = [")
    brk = 10
    for i in range(0,len(mesh.polygons)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.polygons[i].loop_start) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.polygons[*].loop_total = [")
    brk = 10
    for i in range(0,len(mesh.polygons)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.polygons[i].loop_total) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.polygons[*].use_smooth = [")
    brk = 10
    for i in range(0,len(mesh.polygons)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.polygons[i].use_smooth) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.polygons[*].normal[0/1/2] = [")
    brk = 3
    for i in range(0,len(mesh.polygons)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        v = mesh.polygons[i].normal
        f.write("(" + ("%.3f"%v[0]) + ", " + ("%.3f"%v[1]) + ", " + ("%.3f"%v[2]) + "), ")
    f.write("]\n\n")

    f.write("mesh.polygons[*].material_index = [")
    brk = 10
    for i in range(0,len(mesh.polygons)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.polygons[i].material_index) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.loops[*].index = [")
    brk = 10
    for i in range(0,len(mesh.loops)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.loops[i].index) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.loops[*].vertex_index = [")
    brk = 10
    for i in range(0,len(mesh.loops)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.loops[i].vertex_index) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.loops[*].edge_index = [")
    brk = 10
    for i in range(0,len(mesh.loops)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(mesh.loops[i].edge_index) + ", ")
    f.write("]\n\n")
    
    f.write("len(mesh.uv_layers[*].data) = [")
    brk = 10
    for i in range(0,len(mesh.uv_layers)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(len(mesh.uv_layers[i].data)) + ", ")
    f.write("]\n\n")

    f.write("mesh.uv_layers[*].data[*].uv[0/1] = [")
    for i in range(0,len(mesh.uv_layers)):
        f.write("\n    [ ")
        brk = 0
        for v in mesh.uv_layers[i].data:
            if brk == 4:
                f.write("\n      ")
                brk = 0
            brk = brk + 1
            f.write("(" + str("%.3f"%v.uv[0]) + ", " + str("%.3f"%v.uv[1]) + "), ")
        f.write("],")
    f.write(" ]\n\n")
   
    f.write("len(mesh.vertex_colors[*].data) = [")
    brk = 10
    for i in range(0,len(mesh.vertex_colors)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        f.write(str(len(mesh.vertex_colors[i].data)) + ", ")
    f.write("]\n\n")

    f.write("mesh.vertex_colors[*].data[*].color[0/1/2] = [ ")
    for i in range(0,len(mesh.vertex_colors)):
        f.write("\n    [ ")
        brk = 0
        for v in mesh.vertex_colors[i].data:
            if brk == 3:
                f.write("\n      ")
                brk = 0
            brk = brk + 1
            f.write("(" + str("%.3f"%v.color[0]) + ", " + str("%.3f"%v.color[1]) + ", " + str("%.3f"%v.color[2]) + "), ")
        f.write("], ")
    f.write("]\n\n")
    
    f.write("mesh.materials[*].diffuse_color[0/1/2] = [ ")
    brk = 3
    for i in range(0,len(mesh.materials)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        c = mesh.materials[i].diffuse_color
        f.write("(" + ("%.3f"%c[0]) + ", " + ("%.3f"%c[1]) + ", " + ("%.3f"%c[2]) + "), ")
    f.write("]\n\n")
     
    f.write("mesh.materials[*].alpha = [ ")
    brk = 10
    for i in range(0,len(mesh.materials)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        c = mesh.materials[i].alpha
        f.write(("%.3f"%c) + ", ")
    f.write("]\n\n")
    
    f.write("mesh.materials[*].specular_color[0/1/2] = [ ")
    brk = 3
    for i in range(0,len(mesh.materials)):
        if brk == 3:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        c = mesh.materials[i].specular_color
        f.write("(" + ("%.3f"%c[0]) + ", " + ("%.3f"%c[1]) + ", " + ("%.3f"%c[2]) + "), ")
    f.write("]\n\n")
     
    f.write("mesh.materials[*].specular_alpha = [ ")
    brk = 10
    for i in range(0,len(mesh.materials)):
        if brk == 10:
            f.write("\n    ")
            brk = 0
        brk = brk + 1
        c = mesh.materials[i].specular_alpha
        f.write(("%.3f"%c) + ", ")
    f.write("]\n")
    
    f.close()
    
    
    
if len(bpy.context.selected_objects) == 1 and bpy.context.selected_objects[0].type == "MESH":
    test_loop_vertices(bpy.context.selected_objects[0].data,
                       "c:/Users/Marek/root/E2qtgl/data/Blender/export/out/blender_mesh_dump.txt")
else:
    print("ERROR: The script requires exacly one object of the 'MESH' type to be selected.")
