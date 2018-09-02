import bpy
import os

class render_element:
    
    def __init__(
            self,
            vertex_coords,      # A tuple of 3 floats.
            normal_coords,      # A tuple of 3 floats.
            diffuse_colour,     # A tuple of 4 floats all in the range [0,1].
            specular_colour,    # A tuple of 4 floats all in the range [0,1].
            texture_coords      # A list of tuples of 2 floats in the range [0,1] each.
            ):
        self._vertex_coords = (vertex_coords[0],vertex_coords[1],vertex_coords[2])
        self._normal_coords = (normal_coords[0],normal_coords[1],normal_coords[2])
        self._diffuse_colour = (diffuse_colour[0],diffuse_colour[1],diffuse_colour[2],diffuse_colour[3])
        self._specular_colour = (specular_colour[0],specular_colour[1],specular_colour[2],specular_colour[3])
        self._texture_coords = []
        for tc in texture_coords:
            self._texture_coords.append((tc[0],tc[1]))

    def vertex_coords(self):
        return self._vertex_coords
        
    def normal_coords(self):
        return self._normal_coords
        
    def diffuse_colour(self):
        return self._diffuse_colour
        
    def specular_colour(self):
        return self._specular_colour
        
    def num_texture_coords(self):
        return len(self._texture_coords)
        
    def texture_coords(self,index):
        return self._texture_coords[index] 
        
    def __eq__(self,other):
        if (self.vertex_coords() != other.vertex_coords()
            or self.normal_coords() != other.normal_coords()
            or self.diffuse_colour() != other.diffuse_colour()
            or self.specular_colour() != other.specular_colour()
            or self.num_texture_coords() != other.num_texture_coords()):
            return False
        for i in range(0,self.num_texture_coords()):
            if self.texture_coords(i) != other.texture_coords(i):
                return False
        return True

    def __hash__(self):
        result = (hash(self.vertex_coords())
                  +  7 * hash(self.normal_coords())
                  + 17 * hash(self.diffuse_colour())
                  + 29 * hash(self.specular_colour()))
        for i in range(0,self.num_texture_coords()):
            result = result + 101 * hash(self.texture_coords(i))
        return result


class render_buffers:
    def __init__(self):
        self._elements_to_indices = {}
        self._indices_to_elements = {}
        self._triangles = []
        self._counter = 0
        self._num_texture_coordinates = 0
        
    def dump(self):
        print("_elements_to_indices:")
        for E,i in self._elements_to_indices.items():
            print("  { " + str(hash(E)) + ", " + str(i))
        print("_indices_to_elements:")
        for i,E in self._indices_to_elements.items():
            print("  { " + str(i) + ", " + str(hash(E)))
        print("_triangles:")
        for e in self._triangles:
            print("  {" + str(e[0]) + ", " + str(e[1]) + ", " + str(e[2]) + "}")
        print("_counter = " + str(self._counter))
        print("_num_texture_coordinates = " + str(self._num_texture_coordinates))

    def num_elements(self):
        return len(self._elements_to_indices)
        
    def element_at_index(self,index):
        assert index in self._indices_to_elements
        return self._indices_to_elements[index]
        
    def num_triangles(self):
        return len(self._triangles)
        
    def triangle_at_index(self,index):
        assert index < len(self._triangles)
        return self._triangles[index]
        
    def num_texture_coords(self):
        return self._num_texture_coordinates
        
    def add_triangle(
            self,
            A,  # An instance of class 'render_element' representing the first vertex.
            B,  # An instance of class 'render_element' representing the second vertex.
            C   # An instance of class 'render_element' representing the third vertex.
            ):
        self._add_element_if_not_present(A)
        self._add_element_if_not_present(B)
        self._add_element_if_not_present(C)
        self._triangles.append((self._index_of_element(A),
                                self._index_of_element(B),
                                self._index_of_element(C)))

    def add_polygon(
            self,
            polygon     # A list of at least three instances of class 'render_element'.
            ):
        assert len(polygon) >= 3
        for i in range(2,len(polygon)):
            self.add_triangle(polygon[0],polygon[i-1],polygon[i])
                                
    def _add_element_if_not_present(
            self,
            E       # An instance of class 'render_element'.
            ):
        if E not in self._elements_to_indices:
            if self._counter == 0:
                self._num_texture_coordinates = E.num_texture_coords()
            else:
                assert self._num_texture_coordinates == E.num_texture_coords()
            self._elements_to_indices[E] = self._counter
            self._indices_to_elements[self._counter] = E
            self._counter = self._counter + 1

    def _index_of_element(
            self,
            E       # An instance of class 'render_element'.
            ):
        assert E in self._elements_to_indices
        return self._elements_to_indices[E]
        
        
def save_buffers(
        buffers,    # An instance of the class 'render_buffers'.
        root_dir    # A directory into which individual output files will be created.
        ):
    precision_str = "%.6f"
        
    os.makedirs(root_dir, exist_ok=True)

    f = open(os.path.join(root_dir,"indices.txt"),"w")
    f.write("E2::qtgl/buffer/indices/triangles/text\n")
    f.write(str(buffers.num_triangles()) + "\n")
    for i in range(0,buffers.num_triangles()):
        t = buffers.triangle_at_index(i)
        for j in range(0,3):
            assert t[j] >= 0 and t[j] < buffers.num_elements()
            f.write(str(t[j]) + "\n")
    f.close()
        
    f = open(os.path.join(root_dir,"vertices.txt"),"w")
    f.write("E2::qtgl/buffer/vertices/3d/text\n")
    f.write(str(buffers.num_elements()) + "\n")
    for i in range(0,buffers.num_elements()):
        c = buffers.element_at_index(i).vertex_coords()
        f.write((precision_str % c[0]) + "\n")
        f.write((precision_str % c[1]) + "\n")
        f.write((precision_str % c[2]) + "\n")
    f.close()

    f = open(os.path.join(root_dir,"normals.txt"),"w")
    f.write("E2::qtgl/buffer/normals/3d/text\n")
    f.write(str(buffers.num_elements()) + "\n")
    for i in range(0,buffers.num_elements()):
        c = buffers.element_at_index(i).normal_coords()
        f.write((precision_str % c[0]) + "\n")
        f.write((precision_str % c[1]) + "\n")
        f.write((precision_str % c[2]) + "\n")
    f.close()

    f = open(os.path.join(root_dir,"diffuse_colours.txt"),"w")
    f.write("E2::qtgl/buffer/diffuse_colours/text\n")
    f.write(str(buffers.num_elements()) + "\n")
    for i in range(0,buffers.num_elements()):
        c = buffers.element_at_index(i).diffuse_colour()
        f.write((precision_str % c[0]) + "\n")
        f.write((precision_str % c[1]) + "\n")
        f.write((precision_str % c[2]) + "\n")
        f.write((precision_str % c[3]) + "\n")
    f.close()
        
    f = open(os.path.join(root_dir,"specular_colours.txt"),"w")
    f.write("E2::qtgl/buffer/specular_colours/text\n")
    f.write(str(buffers.num_elements()) + "\n")
    for i in range(0,buffers.num_elements()):
        c = buffers.element_at_index(i).specular_colour()
        f.write((precision_str % c[0]) + "\n")
        f.write((precision_str % c[1]) + "\n")
        f.write((precision_str % c[2]) + "\n")
        f.write((precision_str % c[3]) + "\n")
    f.close()

    for i in range(0,buffers.num_texture_coords()):
        f = open(os.path.join(root_dir,"texcoords" + str(i) + ".txt"),"w")
        f.write("E2::qtgl/buffer/texcoords/2d/" + str(i) + "/text\n")
        f.write(str(buffers.num_elements()) + "\n")
        for j in range(0,buffers.num_elements()):
            assert buffers.element_at_index(j).num_texture_coords() > i
            c = buffers.element_at_index(j).texture_coords(i)
            f.write((precision_str % c[0]) + "\n")
            f.write((precision_str % c[1]) + "\n")
        f.close()


def export_mesh(
        mesh,       # An instance of class 'bpy.types.Mesh'
        root_dir    # A directory into which all exported files will be created.
        ):
    buffers = []
    if len(mesh.materials) == 0:
        buffers[0] = render_buffers()
    else:
        for i in range(0,len(mesh.materials)):
            buffers.append(render_buffers())
    for i in range(0,len(mesh.polygons)):
        if len(mesh.materials) == 0:
            mtl_index = 0
        else:
            mtl_index = mesh.polygons[i].material_index
            assert mtl_index < len(mesh.materials)
        polygon = []
        for j in mesh.polygons[i].loop_indices:
            if len(mesh.vertex_colors) > 0:
                colours = []
                for k in range(0,min(2,len(mesh.vertex_colors))):
                    colours.append((
                        mesh.vertex_colors[k].data[j].color[0],
                        mesh.vertex_colors[k].data[j].color[1],
                        mesh.vertex_colors[k].data[j].color[2],
                        1.0
                        ))
                assert len(colors) > 0
                diffuse_colour = colours[0]
                if len(colours) > 1:
                    specular_colour = colours[1]
                else:
                    specular_colour = (0.0, 0.0, 0.0, 1.0)
            elif len(mesh.materials) > 0:
                diffuse_colour = (
                    mesh.materials[mtl_index].diffuse_color[0],
                    mesh.materials[mtl_index].diffuse_color[1],
                    mesh.materials[mtl_index].diffuse_color[2],
                    mesh.materials[mtl_index].alpha
                    )
                specular_colour = (
                    mesh.materials[mtl_index].specular_color[0],
                    mesh.materials[mtl_index].specular_color[1],
                    mesh.materials[mtl_index].specular_color[2],
                    mesh.materials[mtl_index].specular_alpha
                    )
            else:
                diffuse_colour = (0.8, 0.8, 0.8, 1.0)
                specular_colour = (0.0, 0.0, 0.0, 1.0)
            texcoords = []
            for k in range(0,len(mesh.uv_layers)):
                texcoords.append(mesh.uv_layers[k].data[j].uv)
            E = render_element(
                    mesh.vertices[mesh.loops[j].vertex_index].co,
                    mesh.vertices[mesh.loops[j].vertex_index].normal,
                    diffuse_colour,
                    specular_colour,
                    texcoords
                    )
            polygon.append(E)
        buffers[mtl_index].add_polygon(polygon)
    if len(buffers) == 1:
        save_buffers(buffers[0],root_dir)
    else:
        for i in range(0,len(buffers)):
            save_buffers(buffers[i],root_dir + "/" + str(i))


if len(bpy.context.selected_objects) == 1 and bpy.context.selected_objects[0].type == "MESH":
    #print("--------------------------------------------------------------------")
    #mesh = bpy.context.selected_objects[0].data
    #for mat in mesh.materials:
    #    for slot in mat.texture_slots:
    #        if slot is not None:
    #            img = slot.texture.image.filepath
    #            print(img)   
    #for mat in bpy.data.materials:
    #    for slot in mat.texture_slots:
    #        if slot is not None:
    #            img = slot.texture.image
    #            print(img)
    export_mesh(bpy.context.selected_objects[0].data,
                "c:/Users/Marek/root/E2qtgl/data/Blender/export/out_test")
else:
    print("ERROR: The script requires exacly one object of the 'MESH' type to be selected.")



######################################################################################
# import bpy
# import mathutils
# import os
#
# print("----------------------------------------------")
#
# mesh = bpy.context.selected_objects[0].data
# materials = mesh.materials
#
# for mat_idx in range(len(materials)):
#     if materials[mat_idx] is None:
#         continue
#     for slot_idx in range(len(materials[mat_idx].texture_slots)):
#         slot = materials[mat_idx].texture_slots[slot_idx]
#         if slot is None:
#             continue
#
#         texture = slot.texture
#         if not hasattr(texture, "image"):
#             continue
#
#         image = texture.image
#
#         print("materials[mat_idx].name = " + materials[mat_idx].name)
#         print("image.name = " + image.name)
#         print("image.filepath = " + image.filepath)
#
#         old_image_filepath = image.filepath
#
#         image.pack(as_png=True)
#         image.unpack(method="USE_LOCAL")
#
#         print("image.name = " + image.name)
#         print("image.filepath = " + image.filepath)
#
#         image.filepath = old_image_filepath
#
#         print("image.filepath = " + image.filepath)
