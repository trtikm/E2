# This is module registration info for Blender.
bl_info = {
    "name": "E2::qtgl model exporter",
    "author": "E2",
    "version": (1, 0, 0),
    "blender": (2, 76, 0),
    "location": "File > Import-Export",
    "description": "E2::qtgl model exporter. It allows for export of all selected MESH objects "
                   "(bpy.types.Mesh) in the E2::qtgl model format. Exported buffers include: index, "
                   "vertex, normal, colours (diffuse and specular), texture UVs, and bone weights "
                   "(vertex groups). User specifies a root export directory. The base name of the directory "
                   "(i.e. the last part of the pathname) is considered as the model name of the exported "
                   "meshes. All exported buffers are save under that root directory into proper sub-directories "
                   "and files. Each buffer is stored into a separate file. All data are exported in the text mode.",
    "warning": "",
    "wiki_url": "",
    "support": 'COMMUNITY',
    "category": 'Import-Export'
    }


import bpy
import mathutils
import os
import shutil


def from_base_matrix(
        position,       # An instance of 'mathutils.Vector'; i.e. a 3d vector
        orientation     # An instance of 'mathutils.Quaternion'; it must be unit (normalised) quaternion
        ):
    """
    :param position: The origin of the coordinate system.
    :param orientation:  The orientation of axes of the coordinate system.
    :return: An instance of 'mathutils.Matrix' (4x4 matrix) representing a transformation from this
             coordinate system to a parent coordinate system (like world coordinate system)
    """
    return mathutils.Matrix.Translation(position) * orientation.to_matrix().to_4x4()


def to_base_matrix(
        position,       # An instance of 'mathutils.Vector'; i.e. a 3d vector
        orientation     # An instance of 'mathutils.Quaternion'; it must be unit (normalised) quaternion
        ):
    """
    :param position: The origin of the coordinate system.
    :param orientation:  The orientation of axes of the coordinate system.
    :return: An instance of 'mathutils.Matrix' (4x4 matrix) representing a transformation from a parent
             coordinate system (like world coordinate system) into this coordinate system.
    """
    return orientation.to_matrix().transposed().to_4x4() * mathutils.Matrix.Translation(-position)


def is_correct_scale_vector(
        scale_vector    # An instance of 'mathutils.Vector'; i.e. a 3d vector whose coord define scales along axes
        ):
    """
    :param scale_vector: An instance of 'mathutils.Vector'; i.e. a 3d vector whose coord define scales along axes
    :return: The function returns True, if the scale is 1.0 for all three coordinate axes. Otherwise False is returned.
    """
    return abs(1.0 - scale_vector[0]) < 0.001 and\
           abs(1.0 - scale_vector[1]) < 0.001 and\
           abs(1.0 - scale_vector[2]) < 0.001


def does_selected_mesh_object_have_correct_scales(
        obj     # An instance of 'bpy.types.Mesh'
        ):
    """
    This function checks whether none of the exported meshes (and corresponding armatures and bones, if any)
    are without scale (i.e. with scale 1.0 in each axis). This check is necessary,  because the disc format of
    E2::qtgl model do not allow for scale. So, a user must first apply scales to vertices of all exported meshes
    before the exporter can be called.
    :param obj: An instance of 'bpy.types.Mesh'
    :return: The function returns True, if the scale is 1.0 for all three coordinate axes for the passed mesh, and
             related armature and bone objects (if present). Otherwise False is returned.
    """

    assert obj.type == 'MESH'
    if not is_correct_scale_vector(obj.scale):
        print("  ERROR: Mesh '" + obj.name + "' is scaled: " + str(obj.scale) + ".")
        return False
    for mod in obj.modifiers:
        if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
            armature = mod.object
            if not is_correct_scale_vector(armature.scale):
                print("  ERROR: Armature '" + armature.name + "' is scaled: " + str(armature.scale) + ".")
                return False
            # Only pose bones have scale (i.e. bones of the armarute "armature.data.bones" do not have scale)
            for bone in armature.pose.bones:
                if not is_correct_scale_vector(bone.scale):
                    print("  ERROR: Pose bone '" + bone.name + "' of armature '" + armature.name + "' is scaled: " + str(bone.scale) + ".")
                    return False
    return True


class render_element:
    """
    This class represent a render vertex. It consists of coordinates, normal,
    diffuse and specular colours, texture UVs, weights of bones (i.e. the corresponding
    transformation matrices), and indices of transformation matrices (for each bone weight one index)
    """

    def __init__(
            self,
            vertex_coords,          # A tuple of 3 floats.
            normal_coords,          # A tuple of 3 floats.
            diffuse_colour,         # A tuple of 4 floats all in the range [0,1].
            specular_colour,        # A tuple of 4 floats all in the range [0,1].
            texture_coords,         # A list of tuples of 2 floats in the range [0,1] each.
            weights_of_matrices,    # A list of floats
            indices_of_matrices     # A list of ints
            ):
        assert len(weights_of_matrices) == len(indices_of_matrices)
        self._vertex_coords = (vertex_coords[0],vertex_coords[1],vertex_coords[2])
        self._normal_coords = (normal_coords[0],normal_coords[1],normal_coords[2])
        self._diffuse_colour = (diffuse_colour[0],diffuse_colour[1],diffuse_colour[2],diffuse_colour[3])
        self._specular_colour = (specular_colour[0],specular_colour[1],specular_colour[2],specular_colour[3])
        self._texture_coords = []
        for tc in texture_coords:
            self._texture_coords.append((tc[0],tc[1]))
        self._weights_of_matrices = []
        for w in weights_of_matrices:
            self._weights_of_matrices.append(w)
        self._indices_of_matrices = []
        for i in indices_of_matrices:
            self._indices_of_matrices.append(i)

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

    def num_weights_of_matrices(self):
        return len(self._weights_of_matrices)

    def weight_of_matrix(self,index):
        return self._weights_of_matrices[index]

    def num_indices_of_matrices(self):
        return len(self._indices_of_matrices)

    def index_of_matrix(self,index):
        return self._indices_of_matrices[index]

    def __eq__(self,other):
        if (self.vertex_coords() != other.vertex_coords()
            or self.normal_coords() != other.normal_coords()
            or self.diffuse_colour() != other.diffuse_colour()
            or self.specular_colour() != other.specular_colour()
            or self.num_texture_coords() != other.num_texture_coords()
            or self.num_weights_of_matrices() != other.num_weights_of_matrices()
            or self.num_indices_of_matrices() != other.num_indices_of_matrices()):
            return False
        for i in range(0,self.num_texture_coords()):
            if self.texture_coords(i) != other.texture_coords(i):
                return False
        for i in range(0,self.num_weights_of_matrices()):
            if self.weight_of_matrix(i) != other.weight_of_matrix(i):
                return False
        for i in range(0,self.num_indices_of_matrices()):
            if self.index_of_matrix(i) != other.index_of_matrix(i):
                return False
        return True

    def __hash__(self):
        result = (hash(self.vertex_coords())
                  +  7 * hash(self.normal_coords())
                  + 17 * hash(self.diffuse_colour())
                  + 29 * hash(self.specular_colour()))
        for i in range(0,self.num_texture_coords()):
            result = result + 101 * hash(self.texture_coords(i))
        for i in range(0,self.num_weights_of_matrices()):
            result = result + 101 * hash(self.weight_of_matrix(i))
        for i in range(0,self.num_indices_of_matrices()):
            result = result + 101 * hash(self.index_of_matrix(i))
        return result


class render_buffers:
    """
    This object defines a list of triangles. Each triangle consists of 3 indices; each referencing
    instance of the class 'render_element'. Individual instances of 'render_element' class are also
    stored in instance of this class together with the list of triangles. However, due to efficiency
    of searching, these render_elements are stored in two maps: one from elements to indices and the
    other from indices to elements.
    """

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

    def num_weights_of_matrices_per_vertex(self):
        if self.num_elements() == 0:
            return 0
        return self.element_at_index(0).num_weights_of_matrices()

    def num_indices_of_matrices_per_vertex(self):
        if self.num_elements() == 0:
            return 0
        return self.element_at_index(0).num_indices_of_matrices()

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


def build_render_buffers(
        mesh    # An instance of class 'bpy.types.Mesh'
        ):
    """
    The function reads the passed 'mesh' object and builds a list of instances
    of the class 'render_buffers'. In other words it converts the mesh into a list
    of lists of triangles. For each material, there will be created exactly one
    instance 'render_buffers' in the list. It is important to note that indices into the
    list correspond to indices into the list of materials.

    :param mesh: An instance of class 'bpy.types.Mesh'
    :return: The constructed instance of the class 'render_buffers'
    """

    num_weights_per_vertex = 0
    for idx in range(0,len(mesh.vertices)):
        num_weights_per_vertex = max(num_weights_per_vertex,len(mesh.vertices[idx].groups))

    buffers = []
    if len(mesh.materials) == 0:
        buffers.append(render_buffers())
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
                assert len(colours) > 0
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

            vtx = mesh.vertices[mesh.loops[j].vertex_index]

            weights_of_matrices = []
            indices_of_matrices = []
            for i in range(0,num_weights_per_vertex):
                weights_of_matrices.append(0.0)
                indices_of_matrices.append(0)
            for i in range(0,len(vtx.groups)):
                weights_of_matrices[i] = vtx.groups[i].weight
                indices_of_matrices[i] = vtx.groups[i].group
            if num_weights_per_vertex > 1 and abs(sum(weights_of_matrices) - 1.0) > 0.001:
                print("  WARNING: Sum of weights of vertex " + str(mesh.loops[j].vertex_index) + " is " +
                      str(sum(weights_of_matrices)))

            E = render_element(
                    mesh.vertices[mesh.loops[j].vertex_index].co,
                    mesh.vertices[mesh.loops[j].vertex_index].normal,
                    diffuse_colour,
                    specular_colour,
                    texcoords,
                    weights_of_matrices,
                    indices_of_matrices
                    )
            polygon.append(E)
        buffers[mtl_index].add_polygon(polygon)

    return buffers


def save_render_buffers(
        buffers,    # An instance of the class 'render_buffers'.
        output_sub_directory,   # A name of the sub-directory (can be empty) into which all buffers will be saved.
                                # This directory is supposed to distinguish individual 'render_buffers' of one mesh.
                                # It is recommended to pass empty string if the mesh has only one instance of
                                # render_buffers.
        export_info # A dictionary holding properties related to the exported mesh. Both keys and values are strings.
        ):
    """
    The function saves buffers in the passed instance "buffers" of the class 'render_buffers' into disc. Each buffer
    is saved into a separate text file (indeed, we export all data in the textual version of E2::qtgl format).
    The function also extends the passed dictionary "export_info" by the information of saved data - what kind of
    buffers were saved into what files.
    :param buffers: # An instance of the class 'render_buffers'. Its buffers will be saved to the disc.
    :param output_sub_directory: A name of the sub-directory (can be empty) into which all buffers will be saved.
                                 This directory is supposed to distinguish individual 'render_buffers' of one mesh.
                                 It is recommended to pass empty string if the mesh has only one instance of
                                 render_buffers.
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: None
    """

    precision_str = "%.6f"

    mesh_root_dir = os.path.join(
            export_info["root_dir"],
            "meshes",
            export_info["model_name"],
            export_info["mesh_name"],
            output_sub_directory
            )
    os.makedirs(mesh_root_dir, exist_ok=True)

    if "render_buffers" in export_info:
        export_info["render_buffers"].append( {} )
    else:
        export_info["render_buffers"] = [ {} ]

    buffers_export_info = export_info["render_buffers"][-1]

    buffers_export_info["index_buffer"] = os.path.join(mesh_root_dir,"indices.txt")
    with open(buffers_export_info["index_buffer"],"w") as f:
        print("    Saving index buffer: " +
              os.path.relpath(buffers_export_info["index_buffer"],export_info["root_dir"]))
        f.write("E2::qtgl/buffer/indices/triangles/text\n")
        f.write(str(buffers.num_triangles()) + "\n")
        for i in range(0,buffers.num_triangles()):
            t = buffers.triangle_at_index(i)
            for j in range(0,3):
                assert t[j] >= 0 and t[j] < buffers.num_elements()
                f.write(str(t[j]) + "\n")

    buffers_export_info["vertex_buffer"] = os.path.join(mesh_root_dir,"vertices.txt")
    with open(buffers_export_info["vertex_buffer"],"w") as f:
        print("    Saving vertex buffer: " +
              os.path.relpath(buffers_export_info["vertex_buffer"],export_info["root_dir"]))
        f.write("E2::qtgl/buffer/vertices/3d/text\n")
        f.write(str(buffers.num_elements()) + "\n")
        for i in range(0,buffers.num_elements()):
            c = buffers.element_at_index(i).vertex_coords()
            f.write((precision_str % c[0]) + "\n")
            f.write((precision_str % c[1]) + "\n")
            f.write((precision_str % c[2]) + "\n")

    buffers_export_info["normal_buffer"] = os.path.join(mesh_root_dir,"normals.txt")
    with open(buffers_export_info["normal_buffer"],"w") as f:
        print("    Saving normal buffer: " +
              os.path.relpath(buffers_export_info["normal_buffer"],export_info["root_dir"]))
        f.write("E2::qtgl/buffer/normals/3d/text\n")
        f.write(str(buffers.num_elements()) + "\n")
        for i in range(0,buffers.num_elements()):
            c = buffers.element_at_index(i).normal_coords()
            f.write((precision_str % c[0]) + "\n")
            f.write((precision_str % c[1]) + "\n")
            f.write((precision_str % c[2]) + "\n")

    buffers_export_info["diffuse_buffer"] = os.path.join(mesh_root_dir,"diffuse_colours.txt")
    with open(buffers_export_info["diffuse_buffer"],"w") as f:
        print("    Saving diffuse buffer: " +
              os.path.relpath(buffers_export_info["diffuse_buffer"],export_info["root_dir"]))
        f.write("E2::qtgl/buffer/diffuse_colours/text\n")
        f.write(str(buffers.num_elements()) + "\n")
        for i in range(0,buffers.num_elements()):
            c = buffers.element_at_index(i).diffuse_colour()
            f.write((precision_str % c[0]) + "\n")
            f.write((precision_str % c[1]) + "\n")
            f.write((precision_str % c[2]) + "\n")
            f.write((precision_str % c[3]) + "\n")

    buffers_export_info["specular_buffer"] = os.path.join(mesh_root_dir,"specular_colours.txt")
    with open(buffers_export_info["specular_buffer"],"w") as f:
        print("    Saving specular buffer: " +
              os.path.relpath(buffers_export_info["specular_buffer"],export_info["root_dir"]))
        f.write("E2::qtgl/buffer/specular_colours/text\n")
        f.write(str(buffers.num_elements()) + "\n")
        for i in range(0,buffers.num_elements()):
            c = buffers.element_at_index(i).specular_colour()
            f.write((precision_str % c[0]) + "\n")
            f.write((precision_str % c[1]) + "\n")
            f.write((precision_str % c[2]) + "\n")
            f.write((precision_str % c[3]) + "\n")

    buffers_export_info["texcoord_buffers"] = []
    for i in range(0,buffers.num_texture_coords()):
        buffers_export_info["texcoord_buffers"].append(os.path.join(mesh_root_dir,"texcoords" + str(i) + ".txt"))
        with open(buffers_export_info["texcoord_buffers"][-1],"w") as f:
            print("    Saving uv buffer of texture #" + str(i) + ": " +
                  os.path.relpath(buffers_export_info["texcoord_buffers"][-1],export_info["root_dir"]))
            f.write("E2::qtgl/buffer/texcoords/2d/" + str(i) + "/text\n")
            f.write(str(buffers.num_elements()) + "\n")
            for j in range(0,buffers.num_elements()):
                assert buffers.element_at_index(j).num_texture_coords() > i
                c = buffers.element_at_index(j).texture_coords(i)
                f.write((precision_str % c[0]) + "\n")
                f.write((precision_str % c[1]) + "\n")

    assert buffers.num_weights_of_matrices_per_vertex() == buffers.num_indices_of_matrices_per_vertex()
    if buffers.num_weights_of_matrices_per_vertex() > 0:
        buffers_export_info["matrix_weight_buffer"] = os.path.join(mesh_root_dir,"weights_of_matrices.txt")
        with open(buffers_export_info["matrix_weight_buffer"],"w") as f:
            print("    Saving matrix weight buffer: " +
                  os.path.relpath(buffers_export_info["matrix_weight_buffer"],export_info["root_dir"]))
            f.write("E2::qtgl/buffer/weights_of_matrices/" +
                    str(buffers.num_weights_of_matrices_per_vertex()) +
                    "/text\n")
            f.write(str(buffers.num_elements()) + "\n")
            for elem_idx in range(0,buffers.num_elements()):
                elem = buffers.element_at_index(elem_idx)
                for weight_idx in range(0,elem.num_weights_of_matrices()):
                    f.write(str(elem.weight_of_matrix(weight_idx)) + "\n")

        buffers_export_info["matrix_index_buffer"] = os.path.join(mesh_root_dir,"indices_of_matrices.txt")
        with open(buffers_export_info["matrix_index_buffer"],"w") as f:
            print("    Saving matrix index buffer: " +
                  os.path.relpath(buffers_export_info["matrix_index_buffer"],export_info["root_dir"]))
            f.write("E2::qtgl/buffer/indices_of_matrices/" + str(buffers.num_indices_of_matrices_per_vertex()) + "/text\n")
            f.write(str(buffers.num_elements()) + "\n")
            for elem_idx in range(0,buffers.num_elements()):
                elem = buffers.element_at_index(elem_idx)
                for index_idx in range(0,elem.num_indices_of_matrices()):
                    f.write(str(elem.index_of_matrix(index_idx)) + "\n")


def save_textures(
        materials,  # A list of all materials used in an exported mesh
        export_info # A dictionary holding properties related to the exported mesh. Both keys and values are strings.
        ):
    """
    Returns a map from indices of processed materials to a list of path-names of texture files used in the material.
    :param materials: A list of all materials used in an exported mesh.
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: None
    """

    textures_root_dir = os.path.join(export_info["root_dir"],"textures",export_info["model_name"])
    os.makedirs(textures_root_dir, exist_ok=True)

    export_info["images"] = []
    export_info["textures"] = []
    for material in materials:
        images = []
        if material:
            for mtex_slot_index in range(0,len(material.texture_slots)):
                mtex_slot = material.texture_slots[mtex_slot_index]
                if mtex_slot is not None:
                    texture = mtex_slot.texture
                    if hasattr(texture,"image"):
                        image = texture.image
                        images.append(image)

        export_info["images"].append( [] )
        export_info["textures"].append( [] )
        for i in range(0,len(images)):
            image = images[i]
            if os.path.exists(bpy.path.abspath(image.filepath)):
                src_image_pathname = bpy.path.abspath(image.filepath)
            else:
                image.unpack(method="USE_LOCAL")
                src_image_pathname = bpy.path.abspath("//textures/" + image.name)
                assert os.path.exists(src_image_pathname)

            dst_image_name = os.path.splitext(os.path.basename(src_image_pathname))[0]
            dst_image_extension = os.path.splitext(os.path.basename(src_image_pathname))[1]

            images_export_info = export_info["images"][-1]
            images_export_info.append(os.path.join(textures_root_dir,dst_image_name + dst_image_extension))
            print("    Copying image #" + str(i) + ": " +
                  os.path.relpath(images_export_info[-1],export_info["root_dir"]))
            shutil.copyfile(src_image_pathname,images_export_info[-1])

            textures_export_info = export_info["textures"][-1]
            textures_export_info.append(os.path.join(textures_root_dir,dst_image_name + ".txt"))
            with open(textures_export_info[-1],"w") as f:
                print("    Saving texture #" + str(i) + ": " +
                      os.path.relpath(textures_export_info[-1],export_info["root_dir"]))
                f.write("E2::qtgl/texture/text\n")
                f.write("./" + dst_image_name + dst_image_extension + "\n")
                f.write("COMPRESSED_RGBA\n")
                f.write("REPEAT\n")
                f.write("REPEAT\n")
                f.write("LINEAR_MIPMAP_LINEAR\n")
                f.write("LINEAR\n")

        assert len(export_info["images"][-1]) == len(export_info["textures"][-1])


def save_coord_systems_of_bones(
        obj,        # An instance of "bpy.types.Object" with "obj.type == 'MESH'" and "obj.data" being of type
                    # "bpy.type.Mesh".
        export_info # A dictionary holding properties related to the exported mesh. Both keys and values are strings.
        ):
    """
    This function exports coordinate systems of all bones of the armature applied to the passed mesh object "obj".
    The function also updates "export_info" so that "export_info["coord_systems"]" is the pathname of the exported
    file. In case there is no armature applied to the mesh object, this function does nothing (i.e. nothing is exported
    and "export_info" is not modified).
    :param obj: An instance of "bpy.types.Object" with "obj.type == 'MESH'" and "obj.data" being of type
                "bpy.type.Mesh".
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: None
    """

    assert obj.type == 'MESH'

    # We first find an armature applied to the mesh object. It might also be the case there is none.
    armature = None
    for mod in obj.modifiers:
        if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
            armature = mod.object
            break
    if not armature:
        return

    # This is the list into which we store coordinate systems of all bones in the armature
    # in the order as they are listed in the armature (to match their indices in vertex groups).
    coord_systems = []

    # So, let's compute coordinate systems of individual bones.
    for bone in armature.data.bones:

        if not bone.use_deform:
            # The bone is not used for deformation of the child mesh object. It means, that the bone
            # is some "helper" bone, like inverse kinematic bone, so we have to skip it from the export.
            continue

        # First we collect the whole chain of parent bones from the root one to this bone.
        bones_list = []
        xbone = bone
        while xbone:
            bones_list.append(xbone)
            xbone = xbone.parent
        bones_list.reverse()

        # Now we compute "to-space" transformation matrix of the bone.
        # We have to do so by composition with same matrices of all parent bones.
        # And lastly we have to also include the transformation of the mesh into
        # the parent armature.
        transform_matrix = mathutils.Matrix()
        transform_matrix.identity()
        parent_tail = mathutils.Vector((0.0, 0.0, 0.0))
        for xbone in bones_list:
            bone_to_base_matrix = to_base_matrix(parent_tail + xbone.head, xbone.matrix.to_quaternion())
            transform_matrix = bone_to_base_matrix * transform_matrix
            parent_tail = bone_to_base_matrix * (parent_tail + xbone.tail).to_4d()
            parent_tail.resize_3d()
        transform_matrix = transform_matrix * from_base_matrix(obj.location, obj.rotation_quaternion)

        # Finally we compute the position (origin) and rotation (orientation) of the
        # coordinate system of the bone.
        pos = transform_matrix.inverted() * mathutils.Vector((0.0, 0.0, 0.0, 1.0))
        pos.resize_3d()
        rot = transform_matrix.to_3x3().transposed().to_quaternion().normalized()

        # And we store the computed data (i.e. the coordinate system).
        coord_systems.append({ "position": pos, "orientation": rot })

    export_info["coord_systems"] = os.path.join(
        export_info["root_dir"],
        "animation",
        export_info["model_name"],
        export_info["mesh_name"],
        "coord_systems.txt"
        )
    os.makedirs(os.path.dirname(export_info["coord_systems"]), exist_ok=True)
    with open(export_info["coord_systems"],"w") as f:
        print("    Saving coordinate systems: " + os.path.relpath(export_info["coord_systems"],export_info["root_dir"]))
        f.write("E2::qtgl/coordsystems/text\n")
        f.write(str(len(coord_systems)) + "\n")
        for system in coord_systems:
            for i in range(0,3):
                f.write(str(system["position"][i]) + "\n")
            for i in range(0,4):
                f.write(str(system["orientation"][i]) + "\n")


def save_keyframe_coord_systems_of_bones(
        obj,        # An instance of "bpy.types.Object" with "obj.type == 'MESH'" and "obj.data" being of type
                    # "bpy.type.Mesh".
        export_info # A dictionary holding properties related to the exported mesh. Both keys and values are strings.
        ):
    """
    This function exports coordinate systems of all keyframes of an animation of the bones of the armature applied to
    the passed mesh object "obj". The function also updates "export_info" so that
    "export_info["keyframe_coord_systems"]" is a list of the pathnames of the exported files (one file per keyframe).
    In case there is no armature or animation applied to the mesh object, this function does nothing (i.e. nothing
    is exported and "export_info" is not modified).
    :param obj: An instance of "bpy.types.Object" with "obj.type == 'MESH'" and "obj.data" being of type
                "bpy.type.Mesh".
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: None
    """
    assert obj.type == 'MESH'

    # We first find an armature applied to the mesh object. It might also be the case there is none.
    armature = None
    for mod in obj.modifiers:
        if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
            armature = mod.object
            break
    if not armature:
        return
    if not armature.animation_data:
        return
    if not armature.animation_data.action:
        return

    # Before we can start collecting data from the animation, we have to prepare the supporting data structure:
    from_bone_name_to_bone_index = {}
    for idx in range(0, len(armature.data.bones)):
        bone = armature.data.bones[idx]
        assert bone.name not in from_bone_name_to_bone_index
        from_bone_name_to_bone_index[bone.name] = idx

    # First we collect data from individual frames in the animation. We store them into the dictionary "keyframes";
    # keys are time points of the frames and values are arrays of coordinate systems of individual bones relative
    # to their default locations in the armature. Index of a coordinate system in the list relates to the index
    # of the corresponding bone in the armature.
    keyframes = {}

    action = armature.animation_data.action
    start_frame = action.frame_range[0]
    end_frame = action.frame_range[1]

    for fcurve in action.fcurves:
        if fcurve.group:
            bone_name = fcurve.group.name
        else:
            bname_start_idx = fcurve.data_path.index(".bones[\"") + len(".bones[\"")
            bname_end_idx = fcurve.data_path.index("\"].", bname_start_idx)
            bone_name = fcurve.data_path[bname_start_idx:bname_end_idx]

        assert bone_name in from_bone_name_to_bone_index
        bone_index = from_bone_name_to_bone_index[bone_name]
        rotation_mode = armature.pose.bones[bone_index].rotation_mode
        coord_index = fcurve.array_index
        for point in fcurve.keyframe_points:
            frame_number = point.co[0]
            frame_value = point.co[1]

            assert start_frame <= frame_number and frame_number <= end_frame
            # Frame-rate of frames is 25Hz; It is 0.041666666s per frame.
            frame_time_point = (frame_number - start_frame) * 0.041666666

            if frame_time_point not in keyframes:
                # Not animated data should be set to identity (no modification of the default location in the armature).
                # We initialise all data to identity; later some of them might be updated.
                keyframes[frame_time_point] = []
                for idx in range(0,len(armature.data.bones)):
                    keyframes[frame_time_point].append({
                        "position": mathutils.Vector((0.0, 0.0, 0.0)),
                        "orientation": [0.0, 0.0, 0.0, 0.0, rotation_mode]
                        })

            if fcurve.data_path.endswith("location"):
                assert coord_index in [0,1,2]
                keyframes[frame_time_point][bone_index]["position"][coord_index] = frame_value
            else:
                assert coord_index in [0,1,2,3]
                keyframes[frame_time_point][bone_index]["orientation"][coord_index] = frame_value

    for time_point in keyframes.keys():
        for system in keyframes[time_point]:
            if system["orientation"][4] == "QUATERNION":
                system["orientation"] = mathutils.Quaternion((
                    system["orientation"][0],
                    system["orientation"][1],
                    system["orientation"][2],
                    system["orientation"][3]
                    ))
            elif system["orientation"][4] == "AXIS_ANGLE":
                system["orientation"] = mathutils.Quaternion(
                    (system["orientation"][0], system["orientation"][1], system["orientation"][2]),
                    system["orientation"][3]
                    )
            else:
                system["orientation"] = mathutils.Euler(
                    (system["orientation"][0], system["orientation"][1], system["orientation"][2]),
                    system["orientation"][4]
                    ).to_quaternion()

    # We are ready to compute the coordinate systems of bones in individual frames from the collected relative
    # coordinate systems in "keyframes" and from the default coordinate systems of bones in the armature.

    coord_systems_of_frames = {}  # Here we shall store the results

    for time_stamp in keyframes:

        coord_systems_of_frames[time_stamp] = []    # Next we fill in this list by coordinate systems for all bones.

        keyframe = keyframes[time_stamp]
        for bone in armature.data.bones:

            if not bone.use_deform:
                # The bone is not used for deformation of the child mesh object. It means, that the bone
                # is some "helper" bone, like inverse kinematic bone, so we have to skip it from the export.
                continue

            # First we collect the whole chain of parent bones from the root one to this bone.
            bones_list = []
            xbone = bone
            while xbone:
                bones_list.append(xbone)
                xbone = xbone.parent
            bones_list.reverse()

            # Now we compute "to-space" transformation matrix of the bone. We do so by composition of its local
            # coordinate system with the relative coordinate system of the animation and also with the similarly
            # constructed matrices of all parent bones.
            transform_matrix = mathutils.Matrix()
            transform_matrix.identity()
            parent_tail = mathutils.Vector((0.0, 0.0, 0.0))
            for xbone in bones_list:
                bone_to_base_matrix = to_base_matrix(parent_tail + xbone.head,xbone.matrix.to_quaternion())
                xbone_idx = from_bone_name_to_bone_index[xbone.name]
                transform_matrix = to_base_matrix(keyframe[xbone_idx]["position"],keyframe[xbone_idx]["orientation"]) *\
                                   bone_to_base_matrix *\
                                   transform_matrix
                parent_tail = bone_to_base_matrix * (parent_tail + xbone.tail).to_4d()
                parent_tail.resize_3d()

            # Finally we compute the position (origin) and rotation (orientation) of the
            # coordinate system of the bone.
            pos = transform_matrix.inverted() * mathutils.Vector((0.0, 0.0, 0.0, 1.0))
            pos.resize_3d()
            rot = transform_matrix.to_3x3().transposed().to_quaternion().normalized()

            # And we store the computed data (i.e. the coordinate system).
            coord_systems_of_frames[time_stamp].append({ "position": pos, "orientation": rot })

    # It remains to save the computed coordinate systems of bones in individual frames to disc.
    # We store each frame into a separate file. But all files will be written into the same output directory:

    keyframes_output_dir = os.path.join(
        export_info["root_dir"],
        "animation",
        export_info["model_name"],
        export_info["mesh_name"],
        action.name
        )
    os.makedirs(keyframes_output_dir, exist_ok=True)

    export_info["keyframe_coord_systems"] = []  # Here we store pathnames of all saved files.
    frame_idx = 0
    for time_stamp in sorted(coord_systems_of_frames.keys()):
        export_info["keyframe_coord_systems"].append(
            os.path.join(keyframes_output_dir,"keyaframe_" + str(frame_idx) + ".txt")
            )
        with open(export_info["keyframe_coord_systems"][-1],"w") as f:
            print("    Saving keyframe " + str(frame_idx + 1) + "/" + str(len(coord_systems_of_frames))  + ": " +
                  os.path.relpath(export_info["keyframe_coord_systems"][-1],export_info["root_dir"]))
            f.write("E2::qtgl/keyframe/text\n")
            f.write(str(time_stamp) + "\n")
            f.write(str(len(coord_systems_of_frames[time_stamp])) + "\n")
            for system in coord_systems_of_frames[time_stamp]:
                for i in range(0,3):
                    f.write(str(system["position"][i]) + "\n")
                for i in range(0,4):
                    f.write(str(system["orientation"][i]) + "\n")
        frame_idx += 1


def select_best_shaders(
        material_idx,   # An index of material for which the besh shader should be chosen. It is basically an index
                        # into the lists "export_info["render_buffers"]" and "export_info["textures"]".
        export_info   # A dictionary holding properties related to the exported mesh. Both keys and values are strings.
        ):
    """
    The function tries to choose the best pair of vertex and fragment shader w.r.t the exported data (buffers,
    textures, etc.)
    :param material_idx: An index of material for which the besh shader should be chosen. It is basically an index
                         into the lists "export_info["render_buffers"]" and "export_info["textures"]".
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: A pair of names of shaders; the first one is a vertex shader the second one is a fragment shader.
             These names do not comprise path on the disc; they are just plain names of shader files.
    """

    buffers_export_info = export_info["render_buffers"][material_idx]
    textures_export_info = export_info["textures"][material_idx]

    if "matrix_weight_buffer" in buffers_export_info:
        if len(textures_export_info) == 0:
            return "vs_IpUmcOpcFX.txt", "fs_IcFc.txt"
    else:
        if len(textures_export_info) == 0:
            return "vs_IpcUmOpcFc.a=1.txt", "fs_IcFc.txt"
        elif len(textures_export_info) == 1:
            return "vs_IptUmOptF.txt", "fs_ItUdFd.txt"

    return "??", "??" # TODO: Do not know yet :-(


def save_batch_files(
        export_info # A dictionary holding properties related to the exported mesh. Both keys and values are strings.
        ):
    """
    The function saves render batch files for each exported set of data (for each material one set of data).
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: None
    """

    batch_root_dir = os.path.join(export_info["root_dir"],"models",export_info["model_name"])
    os.makedirs(batch_root_dir, exist_ok=True)

    export_info["batch_files"] = []
    for material_idx in range(0,len(export_info["render_buffers"])):
        export_info["batch_files"].append(os.path.join(batch_root_dir,export_info["mesh_name"] + ".txt"))
        with open(export_info["batch_files"][-1],"w") as f:
            print("    Saving batch: " +
                  os.path.relpath(export_info["batch_files"][-1],export_info["root_dir"]))
            f.write("E2::qtgl/batch/indexed/text\n")

            buffers_export_info = export_info["render_buffers"][material_idx]
            textures_export_info = export_info["textures"][material_idx]

            vertex_shader, fragment_shader = select_best_shaders(material_idx,export_info)
            f.write(os.path.join("..","..","shaders","vertex",vertex_shader) + "\n")
            f.write(os.path.join("..","..","shaders","fragment",fragment_shader) + "\n")

            f.write(os.path.relpath(buffers_export_info["index_buffer"],batch_root_dir) + "\n")
            f.write("BINDING_IN_POSITION\n")
            f.write(os.path.relpath(buffers_export_info["vertex_buffer"],batch_root_dir) + "\n")
            for texcoords_idx in range(0,len(buffers_export_info["texcoord_buffers"])):
                f.write("BINDING_IN_TEXCOORD" + str(texcoords_idx) + "\n")
                f.write(os.path.relpath(buffers_export_info["texcoord_buffers"][texcoords_idx],batch_root_dir) + "\n")

            if len(buffers_export_info["texcoord_buffers"]) == 0:
                f.write("BINDING_IN_COLOUR\n")
                f.write(os.path.relpath(buffers_export_info["diffuse_buffer"],batch_root_dir) + "\n")

            texture_binding_names = [
                "BINDING_TEXTURE_DIFFUSE",
                "BINDING_TEXTURE_NORMALS"
                ]
            for i in range(0,len(textures_export_info)):
                if i < len(texture_binding_names):
                    f.write(texture_binding_names[i] + "\n")
                else:
                    f.write("BINDING_TEXTURE_??\n")
                f.write(os.path.relpath(textures_export_info[i],batch_root_dir) + "\n")

            if "matrix_weight_buffer" in buffers_export_info:
                assert "matrix_index_buffer" in buffers_export_info
                f.write("BINDING_IN_INDICES_OF_MATRICES\n")
                f.write(os.path.relpath(buffers_export_info["matrix_index_buffer"],batch_root_dir) + "\n")
                f.write("BINDING_IN_WEIGHTS_OF_MATRICES\n")
                f.write(os.path.relpath(buffers_export_info["matrix_weight_buffer"],batch_root_dir) + "\n")

            f.write("BACK\n")
            f.write("false\n")


def export_selected_meshes(
        export_dir      # A directory under which all exported files will be saved.
        ):
    """
    This is the root function of the whole export. Its purpose is to check whether data to be exported satisfies
    requirement (like no-scale rule), and then manages the export process for each selected mesh object.
    :param export_dir: A directory under which all exported files will be saved.
    :return: None
    """
    print("Starting E2::qtgl model exporter.")

    os.makedirs(export_dir, exist_ok=True)

    model_name = os.path.basename(export_dir)
    if len(model_name) > 0:
        print("  Model name for the selected meshes: " + model_name)
        print("  Root output directory: " + export_dir)
        mesh_names = set()
        for i in range(0,len(bpy.context.selected_objects)):
            obj = bpy.context.selected_objects[i]
            if obj.type != 'MESH':
                print("  ERROR: Object '" + obj.name + "' is not a MESH. Skipping it...")
                continue
            if obj.data.name is None or len(obj.data.name) == 0:
                print("  ERROR: The selected mesh object has invalid name '" + obj.data.name + "'. Skipping it...")
                continue
            if not does_selected_mesh_object_have_correct_scales(obj):
                continue
            if len(obj.data.vertex_colors) > 2:
                print("  ERROR: The mesh object '" + obj.name + "' has more than 2 colours per vertex. "
                      "Skipping it...")
                continue
            num_armatures = 0
            armature = None
            for mod in obj.modifiers:
                if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
                    armature = mod.object
                    num_armatures += 1
            if num_armatures > 1:
                print("  ERROR: The mesh object '" + obj.name + "' has more than one armature. Skipping it...")
                continue
            if num_armatures == 1 and len(armature.data.bones) == 0:
                print("  ERROR: The armature '" + armature.name + "' of the object '" + obj.name + "'has no bone."
                      " Skipping the object...")
                continue

            mesh = obj.data

            print("  Exporting mesh: " + mesh.name)

            mesh_name = mesh.name
            while mesh_name in mesh_names:
                mesh_name = mesh_name + str(i)
            mesh_names.add(mesh_name)

            export_info = {
                "model_name": model_name,
                "mesh_name": mesh_name,
                "root_dir": export_dir
            }

            buffers_list = build_render_buffers(mesh)
            for idx in range(0,len(buffers_list)):
                sub_directory = ""
                if len(buffers_list) > 1:
                    sub_directory = sub_directory + str(idx)
                save_render_buffers(buffers_list[idx],sub_directory,export_info)

            save_textures(mesh.materials,export_info)

            assert len(export_info["render_buffers"]) == len(export_info["images"])
            assert len(export_info["render_buffers"]) == len(export_info["textures"])

            save_coord_systems_of_bones(obj,export_info)
            save_keyframe_coord_systems_of_bones(obj,export_info)

            save_batch_files(export_info)
    else:
        print("  ERROR: Cannot deduce model name from the passed export directory '" + export_dir + "'.")

    print("Terminating E2::qtgl model exporter.")


class E2_model_exporter(bpy.types.Operator):
    """
    This class defines behaviour of this plug in the export menu of Blender.
    It means that it enables the export, if one or more mesh objects are selected.
    Then, if the export is enabled, it asks a user to provide an export root directory
    under which all data files in E2::qtgl format will be stored, and then calls the
    function 'export_selected_meshes()' with that directory. The function then does the
    whole export.
    """
    bl_idname = "export.e2_qtgl_model_exporter"
    bl_label = "E2::qtgl model exporter"
    directory = bpy.props.StringProperty(
                    name="An output directory",
                    description="A root directory into which all data will be exported.",
                    subtype="DIR_PATH",
                    maxlen= 1024,
                    default= "")

    @classmethod
    def poll(cls, context):
        if len(bpy.context.selected_objects) < 1:
            return False
        for i in range(0,len(bpy.context.selected_objects)):
            if bpy.context.selected_objects[i].type != "MESH":
                return False
        return True

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def execute(self, context):
        export_selected_meshes(os.path.normpath(self.directory))
        return{'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(E2_model_exporter.bl_idname, text="E2::qtgl model exporter")


def register():
    bpy.utils.register_class(E2_model_exporter)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(E2_model_exporter)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    export_selected_meshes("c:/Users/Marek/root/E2/temp/!MODELS/barbarian_female/barbarian_female")
    #register()
