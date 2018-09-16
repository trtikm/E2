# This is module registration info for Blender.
bl_info = {
    "name": "E2 gfx exporter",
    "author": "E2",
    "version": (1, 0, 0),
    "blender": (2, 76, 0),
    "location": "File > Import-Export",
    "description": "Exports gfx data of a selected objects, like mesh or armature, in E2 gfx format - a hierarchy of "
                   "directories and files under a given root output directory. Names of created "
                   "files and directories define semantics of the exported data.",
    "warning": "",
    "wiki_url": "",
    "support": 'COMMUNITY',
    "category": 'Import-Export'
    }


import bpy
import mathutils
import math
import time
import os
import shutil
import json
import traceback


class TimeProf:
    """
    This is a time profiler. Singleton.
    """

    _instance = None

    @staticmethod
    def instance():
        if TimeProf._instance is None:
            TimeProf._instance = TimeProf()
        return TimeProf._instance

    def __init__(self):
        self._statistics = {}
        self._stack = []

    def start(self, measured_block_id):
        assert isinstance(measured_block_id, str) and len(measured_block_id) > 0
        self._stack.append((measured_block_id, time.time()))
        return self

    def stop(self):
        assert len(self._stack) > 0
        duration = time.time() - self._stack[-1][1]
        if self._stack[-1][0] not in self._statistics:
            self._statistics[self._stack[-1][0]] = {
                "total_time": 0.0,
                "hit_count": 0,
                "max_time": 0.0
            }
        record = self._statistics[self._stack[-1][0]]
        record["total_time"] += duration
        record["hit_count"] += 1
        record["max_time"] = duration if duration > record["max_time"] else record["max_time"]
        self._stack.pop()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()

    def to_json(self):
        return json.dumps(self._statistics, indent=4, sort_keys=True)


def get_number_precision_string():
    return ".6f"


def float_to_string(number):
    assert isinstance(number, float)
    return format(number, get_number_precision_string())


def remove_ignored_part_of_name(name, phase=None):
    assert isinstance(name, str)
    assert phase is None or isinstance(phase, str)

    def process_name(name, phase):
        if "${" not in name:
            return [name]
        prefix = name[:name.find("${")]
        rest = name[name.find("${") + 2:]
        if "}" not in rest:
            return [name]
        command = rest[:rest.find("}")]
        if command == "IGNOREME":
            command = "IGNOREME:"
        suffix = rest[rest.find("}") + 1:]
        if ":" not in command:
            return [name]
        where_use = command[:rest.find(":")]
        what_use = command[rest.find(":") + 1:]
        if where_use == "IGNOREME" or phase is None or phase not in where_use:
            what_use = ""
        return [prefix] + [what_use] + process_name(suffix, phase)

    return "".join(process_name(name, phase))


def vector2_neg(u):
    return -u[0], -u[1]


def vector2_add(u, v):
    return u[0] + v[0], u[1] + v[1]


def vector2_mul(a, u):
    return a * u[0], a * u[1]


def vector2_sub(u, v):
    return vector2_add(u, vector2_neg(v))


def vector2_dot(u, v):
    return u[0] * v[0] + u[1] * v[1]


def vector2_ortho(u):
    return -u[1], u[0]


def vector3_zero():
    return 0.0, 0.0, 0.0


def vector3_axis_x():
    return 1.0, 0.0, 0.0


def vector3_axis_y():
    return 0.0, 1.0, 0.0


def vector3_axis_z():
    return 0.0, 0.0, 1.0


def vector3_neg(u):
    return -u[0], -u[1], -u[2]


def vector3_add(u, v):
    return u[0] + v[0], u[1] + v[1], u[2] + v[2]


def vector3_mul(a, u):
    return a * u[0], a * u[1], a * u[2]


def vector3_sub(u, v):
    return vector3_add(u, vector3_neg(v))


def vector3_dot(u, v):
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2]


def vector3_cross(u, v):
    return u[1] * v[2] - v[1] * u[2],  u[2] * v[0] - v[2] * u[0], u[0] * v[1] - v[0] * u[1]


def vector3_length(u):
    return math.sqrt(vector3_dot(u, u))


def vector3_normalised(u):
    return vector3_mul(1.0 / vector3_length(u), u)


def vector3_perpendicular(u, v):
    """ Returns a vector 'w' perpendicular to 'v' lying also in the plane 'X = p*u + q*v', for any 'p,q'. """
    return vector3_sub(u, vector3_mul(vector3_dot(u, v) / vector3_dot(v, v), v))


def vector3_any_linear_independent(u):
    return sorted([(vector3_axis_x(), abs(u[0])), (vector3_axis_y(), abs(u[1])), (vector3_axis_z(), abs(u[2]))],
                  key=lambda x: x[1])[0][0]


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
    with TimeProf.instance().start("to_base_matrix"):
        return orientation.to_matrix().transposed().to_4x4() * mathutils.Matrix.Translation(-position)


def compute_tangent_vector_of_textured_triangle(A, B, C, uvA, uvB, uvC):
    u = vector2_sub(uvB, uvA)
    v = vector2_sub(uvC, uvA)
    denom = vector2_dot(u, vector2_ortho(v))
    min_denom = 0.00000001
    if abs(denom) < min_denom:
        return None
    return vector3_normalised(vector3_add(vector3_mul(-v[1] / denom, vector3_sub(B, A)),
                                          vector3_mul( u[1] / denom, vector3_sub(C, A))))


def get_object_rotation_quaternion(obj):
    """
    :param obj: Any instance of 'bpy.types.Object'.
    :return: A quaternion representing rotation of the passed object. If the object uses some other kind of
             rotation representation, then there is applied a conversion.
    """
    with TimeProf.instance().start("get_object_rotation_quaternion"):
        if obj.rotation_mode == 'QUATERNION':
            return obj.rotation_quaternion
        elif obj.rotation_mode == "AXIS_ANGLE":
            return mathutils.Quaternion(
                (obj.rotation_axis_angle[0], obj.rotation_axis_angle[1], obj.rotation_axis_angle[2]),
                obj.rotation_axis_angle[3]
                )
        else:
            return mathutils.Euler(
                (obj.rotation_euler[0], obj.rotation_euler[1], obj.rotation_euler[2]),
                obj.rotation_mode
                ).to_quaternion()


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


def armature_has_correct_scales(
        armature    # An instance of 'bpy.types.Armature'
        ):
    """
    This function checks whether the passed armatue or its pose bones are without scale (i.e. with scale 1.0 in each
    axis). This check is necessary,  because the E2 format does not allow for scale. So, a user must first apply scales
    to vertices of all exported meshes before the exporter can be called.
    :param obj: An instance of 'bpy.types.Armature'
    :return: The function returns True, if the scale is 1.0 for all three coordinate axes for the passed armature and
             all its pose bones. Otherwise False is returned.
    """

    assert armature.type == "ARMATURE"

    with TimeProf.instance().start("armature_has_correct_scales"):
        if not is_correct_scale_vector(armature.scale):
            print("ERROR: The armature is scaled: " + str(armature.scale) + ".")
            return False
        # Only pose bones have scale (i.e. bones of the armature "armature.data.bones" do not have scale)
        for bone in armature.pose.bones:
            if not is_correct_scale_vector(bone.scale):
                print("  ERROR: Pose bone '" + bone.name + "' of the armature is scaled: " + str(bone.scale) + ".")
                return False
        return True


def vertices_have_correct_weights(
        vertices     # A list of vertices obtained from a 'bpy.types.Mesh' instance
        ):
    """
    This function checks whether all vertices have correct number of wights, i.e. at most 4.
    It also checks whether the weights are normalised. i.e. their sum is equal to 1, for each vertex.
    :param obj: An instance of 'bpy.types.Mesh'
    :return: The function returns True, if each vertex has correct at most 4 normalised weights.
             Otherwise False is returned.
    """

    with TimeProf.instance().start("vertices_have_correct_weights"):
        for idx in range(0, len(vertices)):
            num_weights = len(vertices[idx].groups)
            if num_weights > 4:
                print("ERROR: The vertex no." + str(idx) + " of the mesh has more than 4 weights, namely " +
                      str(num_weights) + ". You can fix the issue by 1. switching to 'Wight Paint' mode "
                     "(select armature, then mesh, and press Ctrl+TAB), 2. Select menu option 'Weights/Limit Total'.")
                return False
            if num_weights > 1:
                sum_of_weights = 0
                for i in range(0, num_weights):
                    sum_of_weights += vertices[idx].groups[i].weight
                if abs(sum_of_weights - 1.0) > 0.001:
                    print("ERROR: Sum of weights of vertex " + str(idx) + " of the mesh is " +
                          str(sum_of_weights) + ". To normalise the weights (so that the sum is 1) do this: 1. "
                          "switching to 'Wight Paint' mode (select armature, then mesh, and press Ctrl+TAB), 2. Select "
                          "menu option 'Weights/Normalize All'.")
                    return False
        return True


def does_vertex_groups_names_match_names_of_bones(
        obj,        # An instance of "bpy.types.Object" with "obj.type == 'MESH'" and "obj.data" being of type
                    # "bpy.type.Mesh".
        armature
        ):

    with TimeProf.instance().start("does_vertex_groups_names_match_names_of_bones"):
        if not armature:
            return True

        bone_names = set()
        for bone in armature.data.bones:
            if bone.use_deform:
                bone_names.add(bone.name)
        for grp in obj.vertex_groups:
            if grp.name not in bone_names:
                print("ERROR: The name '" + grp.name + "' of vertex group does not match the name of any bone. NOTE: "
                      "We assume that names of vertex groups match the names of the corresponding bones. ")
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
        self._tangent_coords = (0.0,0.0,0.0)
        self._bitangent_coords = (0.0,0.0,0.0)
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

    def tangent_coords(self):
        return self._tangent_coords

    def bitangent_coords(self):
        return self._bitangent_coords

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

    def set_tangent_coords(self, tangent_coords):
        self._tangent_coords = tangent_coords

    def set_bitangent_coords(self, bitangent_coords):
        self._bitangent_coords = bitangent_coords

    def __eq__(self,other):
        with TimeProf.instance().start("render_element.__eq__"):
            if (self.vertex_coords() != other.vertex_coords()
                or self.normal_coords() != other.normal_coords()
                or self.tangent_coords() != other.tangent_coords()
                or self.bitangent_coords() != other.bitangent_coords()
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
        with TimeProf.instance().start("render_element.__hash__"):
            result = (hash(self.vertex_coords())
                      +  7 * hash(self.normal_coords())
                      + 31 * hash(self.tangent_coords())
                      + 101 * hash(self.bitangent_coords())
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
        self._has_tangent_space = False

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

    def has_tangent_space(self):
        return self._has_tangent_space

    def num_weights_of_matrices_per_vertex(self):
        if self.num_elements() == 0:
            return 0
        return self.element_at_index(0).num_weights_of_matrices()

    def num_indices_of_matrices_per_vertex(self):
        if self.num_elements() == 0:
            return 0
        return self.element_at_index(0).num_indices_of_matrices()

    def add_polygon(
            self,
            polygon     # A list of at least three instances of class 'render_element'.
            ):
        assert len(polygon) >= 3
        indices = [self._add_element_if_not_present(E) for E in polygon]
        for i in range(2,len(indices)):
            self._triangles.append((indices[0], indices[i-1], indices[i]))

    def _add_element_if_not_present(
            self,
            E       # An instance of class 'render_element'.
            ):
        index = self._elements_to_indices.get(E)
        if index is None:
            if self._counter == 0:
                self._num_texture_coordinates = E.num_texture_coords()
            else:
                assert self._num_texture_coordinates == E.num_texture_coords()
            self._elements_to_indices[E] = self._counter
            self._indices_to_elements[self._counter] = E
            index = self._counter
            self._counter += 1
        return index

    def compute_tangent_space(self):
        if self.num_elements() == 0 or self.num_texture_coords() == 0:
            return
        texcoord_idx = 2 if 2 < self.num_texture_coords() else 0    # 0 - diffuse (or default), 1 - specular, 2 - normal
        for triangle_idx in range(self.num_triangles()):
            triangle = self.triangle_at_index(triangle_idx)
            render_elems = [self.element_at_index(triangle[i]) for i in [0, 1, 2, 0, 1]]
            points = [elem.vertex_coords() for elem in render_elems]
            texels = [elem.texture_coords(texcoord_idx) for elem in render_elems]
            for start_point_idx in range(3):
                tangent = compute_tangent_vector_of_textured_triangle(
                                points[start_point_idx + 0],
                                points[start_point_idx + 1],
                                points[start_point_idx + 2],
                                texels[start_point_idx + 0],
                                texels[start_point_idx + 1],
                                texels[start_point_idx + 2]
                                )
                elem = render_elems[start_point_idx]
                if tangent is not None:
                    tangent = vector3_normalised(vector3_perpendicular(tangent, elem.normal_coords()))
                    elem.set_tangent_coords(vector3_add(elem.tangent_coords(), tangent))
                else:
                    print("WARNING: Degenerated uv-triangle detected:")
                    for name, idx in [("A", 0), ("B", 1), ("C", 2)]:
                        print("         " + name + "=" + str(points[start_point_idx + idx]) + ", uv=" + str(texels[start_point_idx + idx]))
                    print("         Skipping tangent computation.")
        for elem_idx in range(self.num_elements()):
            elem = self.element_at_index(elem_idx)
            tangent = vector3_perpendicular(elem.tangent_coords(), elem.normal_coords())
            if vector3_length(tangent) < 0.001:
                print("WARNING: The main algorithm for tangent space computation has FAILED at point:")
                print("         " + str(elem.vertex_coords()))
                print("         Generation a randomly chosen tangent space at that point.")
                tangent = vector3_perpendicular(vector3_any_linear_independent(elem.normal_coords()), elem.normal_coords())
            tangent = vector3_normalised(tangent)
            bitangent = vector3_cross(elem.normal_coords(), tangent)
            elem.set_tangent_coords(tangent)
            elem.set_bitangent_coords(bitangent)
        self._has_tangent_space = True


def build_render_buffers(
        obj,        # An instance of "bpy.types.Object" with "obj.type == 'MESH'" and "obj.data" being of type
                    # "bpy.type.Mesh".
        armature    # An armature deforming the mesh. It can be None (when no such armature exists)
        ):
    """
    The function reads the passed 'mesh' object and builds a list of instances
    of the class 'render_buffers'. In other words it converts the mesh into a list
    of lists of triangles. For each material, there will be created exactly one
    instance 'render_buffers' in the list. It is important to note that indices into the
    list correspond to indices into the list of materials.

    :param mesh: An instance of class 'bpy.types.Mesh'
    :param armature: An armature deforming the mesh. It can be None (when no such armature exists)
    :return: The constructed instance of the class 'render_buffers'
    """

    with TimeProf.instance().start("build_render_buffers"):
        mesh = obj.data

        if not armature:
            deform_bone_index = None
        else:
            deform_bone_index = {}
            deform_bones_counter = 0
            for idx in range(len(armature.data.bones)):
                bone = armature.data.bones[idx]
                if bone.use_deform:
                    deform_bone_index[bone.name] = deform_bones_counter
                    deform_bones_counter += 1

        num_weights_per_vertex = 0
        for idx in range(len(mesh.vertices)):
            num_weights_per_vertex = max(num_weights_per_vertex,len(mesh.vertices[idx].groups))

        buffers = []
        if len(mesh.materials) == 0:
            buffers.append(render_buffers())
        else:
            for i in range(len(mesh.materials)):
                buffers.append(render_buffers())

        for i in range(len(mesh.polygons)):
            if len(mesh.materials) == 0:
                mtl_index = 0
            else:
                mtl_index = mesh.polygons[i].material_index
                assert mtl_index < len(mesh.materials)
            polygon = []
            for j in mesh.polygons[i].loop_indices:
                if len(mesh.vertex_colors) > 0:
                    colours = []
                    for k in range(min(2, len(mesh.vertex_colors))):
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
                for k in range(len(mesh.uv_layers)):
                    texcoords.append(mesh.uv_layers[k].data[j].uv)

                vtx = mesh.vertices[mesh.loops[j].vertex_index]

                assert len(vtx.groups) == 0 or deform_bone_index is not None

                weights_of_matrices = []
                indices_of_matrices = []
                for _ in range(num_weights_per_vertex):
                    weights_of_matrices.append(0.0)
                    indices_of_matrices.append(0)
                for grp_idx in range(len(vtx.groups)):
                    weights_of_matrices[grp_idx] = vtx.groups[grp_idx].weight
                    indices_of_matrices[grp_idx] = deform_bone_index[obj.vertex_groups[vtx.groups[grp_idx].group].name]
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

        for mtl_index in range(len(buffers)):
            if mtl_index < len(mesh.materials) and mesh.materials[mtl_index] is not None:
                material = mesh.materials[mtl_index]
                has_normal_map = False
                for slot_idx in range(len(material.texture_slots)):
                    slot = material.texture_slots[slot_idx]
                    if slot is not None and slot.use_map_normal:
                        has_normal_map = True
                        break
                if has_normal_map is True:
                    buffers[mtl_index].compute_tangent_space()

        return buffers


def save_render_buffers(
        buffers,        # An instance of the class 'render_buffers'.
        material_name,  # Material name of that part of the exported mash having the material.
        armature_name,  # Name of the armature deforming the mesh. It can be None (when no armature exists).
        export_info     # A dictionary holding properties related to the export.
        ):
    """
    The function saves buffers in the passed instance "buffers" of the class 'render_buffers' to disk. Each buffer
    is saved into a separate E2 text file. The function also extends the passed dictionary 'export_info' by the
    information of saved data - what kind of buffers were saved into what files.
    :param buffers: # An instance of the class 'render_buffers'. Its buffers will be saved to the disk.
    :param material_name: Material name of that part of the exported mash having the material.
    :param export_info: A dictionary holding properties related to the export.
    :return: None
    """

    assert material_name is None or (isinstance(material_name, str) and len(material_name) > 0)

    with TimeProf.instance().start("save_render_buffers"):
        mesh_root_dir = os.path.join(
            export_info["root_dir"],
            "meshes",
            export_info["mesh_name"],
            material_name if material_name is not None else ""
            )
        os.makedirs(mesh_root_dir, exist_ok=True)

        if "render_buffers" in export_info:
            export_info["render_buffers"].append({})
        else:
            export_info["render_buffers"] = [{}]

        buffers_export_info = export_info["render_buffers"][-1]

        buffers_root_dir = os.path.join(mesh_root_dir, "buffers")
        os.makedirs(buffers_root_dir, exist_ok=True)

        buffers_export_info["root_dir"] = buffers_root_dir
        buffers_export_info["mesh_root_dir"] = mesh_root_dir

        buffers_export_info["index_buffer"] = os.path.join(buffers_root_dir, "indices.txt")
        with open(buffers_export_info["index_buffer"], "w") as f:
            print("Saving index buffer: " +
                  os.path.relpath(buffers_export_info["index_buffer"], export_info["root_dir"]))
            f.write("3\n")
            f.write(str(buffers.num_triangles()) + "\n")
            for i in range(buffers.num_triangles()):
                t = buffers.triangle_at_index(i)
                for j in range(3):
                    assert t[j] >= 0 and t[j] < buffers.num_elements()
                    f.write(str(t[j]) + "\n")

        buffers_export_info["vertex_buffer"] = os.path.join(buffers_root_dir, "vertices.txt")
        with open(buffers_export_info["vertex_buffer"], "w") as f:
            print("Saving vertex buffer: " +
                  os.path.relpath(buffers_export_info["vertex_buffer"], export_info["root_dir"]))
            f.write(str(buffers.num_elements()) + "\n")
            for i in range(buffers.num_elements()):
                c = buffers.element_at_index(i).vertex_coords()
                f.write(float_to_string(c[0]) + "\n")
                f.write(float_to_string(c[1]) + "\n")
                f.write(float_to_string(c[2]) + "\n")

        buffers_export_info["normal_buffer"] = os.path.join(buffers_root_dir, "normals.txt")
        with open(buffers_export_info["normal_buffer"], "w") as f:
            print("Saving normal buffer: " +
                  os.path.relpath(buffers_export_info["normal_buffer"], export_info["root_dir"]))
            f.write(str(buffers.num_elements()) + "\n")
            for i in range(buffers.num_elements()):
                c = buffers.element_at_index(i).normal_coords()
                f.write(float_to_string(c[0]) + "\n")
                f.write(float_to_string(c[1]) + "\n")
                f.write(float_to_string(c[2]) + "\n")

        if buffers.has_tangent_space():
            buffers_export_info["tangent_buffer"] = os.path.join(buffers_root_dir, "tangents.txt")
            with open(buffers_export_info["tangent_buffer"], "w") as f:
                print("Saving tangent buffer: " +
                      os.path.relpath(buffers_export_info["tangent_buffer"], export_info["root_dir"]))
                f.write(str(buffers.num_elements()) + "\n")
                for i in range(buffers.num_elements()):
                    c = buffers.element_at_index(i).tangent_coords()
                    f.write(float_to_string(c[0]) + "\n")
                    f.write(float_to_string(c[1]) + "\n")
                    f.write(float_to_string(c[2]) + "\n")

            buffers_export_info["bitangent_buffer"] = os.path.join(buffers_root_dir, "bitangents.txt")
            with open(buffers_export_info["bitangent_buffer"], "w") as f:
                print("Saving bitangent buffer: " +
                      os.path.relpath(buffers_export_info["bitangent_buffer"], export_info["root_dir"]))
                f.write(str(buffers.num_elements()) + "\n")
                for i in range(buffers.num_elements()):
                    c = buffers.element_at_index(i).bitangent_coords()
                    f.write(float_to_string(c[0]) + "\n")
                    f.write(float_to_string(c[1]) + "\n")
                    f.write(float_to_string(c[2]) + "\n")

        buffers_export_info["diffuse_buffer"] = os.path.join(buffers_root_dir, "diffuse.txt")
        with open(buffers_export_info["diffuse_buffer"], "w") as f:
            print("Saving diffuse buffer: " +
                  os.path.relpath(buffers_export_info["diffuse_buffer"], export_info["root_dir"]))
            f.write("4\n")
            f.write(str(buffers.num_elements()) + "\n")
            for i in range(buffers.num_elements()):
                c = buffers.element_at_index(i).diffuse_colour()
                f.write(float_to_string(c[0]) + "\n")
                f.write(float_to_string(c[1]) + "\n")
                f.write(float_to_string(c[2]) + "\n")
                f.write(float_to_string(c[3]) + "\n")

        buffers_export_info["specular_buffer"] = os.path.join(buffers_root_dir, "specular.txt")
        with open(buffers_export_info["specular_buffer"], "w") as f:
            print("Saving specular buffer: " +
                  os.path.relpath(buffers_export_info["specular_buffer"], export_info["root_dir"]))
            f.write("4\n")
            f.write(str(buffers.num_elements()) + "\n")
            for i in range(buffers.num_elements()):
                c = buffers.element_at_index(i).specular_colour()
                f.write(float_to_string(c[0]) + "\n")
                f.write(float_to_string(c[1]) + "\n")
                f.write(float_to_string(c[2]) + "\n")
                f.write(float_to_string(c[3]) + "\n")

        buffers_export_info["texcoord_buffers"] = []
        for i in range(buffers.num_texture_coords()):
            buffers_export_info["texcoord_buffers"].append(os.path.join(buffers_root_dir, "texcoords" + str(i) + ".txt"))
            with open(buffers_export_info["texcoord_buffers"][-1],"w") as f:
                print("Saving uv buffer of texture #" + str(i) + ": " +
                      os.path.relpath(buffers_export_info["texcoord_buffers"][-1], export_info["root_dir"]))
                f.write(str(buffers.num_elements()) + "\n")
                for j in range(buffers.num_elements()):
                    assert buffers.element_at_index(j).num_texture_coords() > i
                    c = buffers.element_at_index(j).texture_coords(i)
                    f.write(float_to_string(c[0]) + "\n")
                    f.write(float_to_string(c[1]) + "\n")

        assert buffers.num_weights_of_matrices_per_vertex() == buffers.num_indices_of_matrices_per_vertex()
        if buffers.num_weights_of_matrices_per_vertex() > 0:
            assert armature_name is not None and isinstance(armature_name, str) and len(armature_name) > 0

            skeletal_root_dir = os.path.join(mesh_root_dir, "skeletal", armature_name)
            os.makedirs(skeletal_root_dir, exist_ok=True)

            buffers_export_info["skeletal_root_dir"] = skeletal_root_dir

            buffers_export_info["matrix_weight_buffer"] = os.path.join(skeletal_root_dir, "weights.txt")
            with open(buffers_export_info["matrix_weight_buffer"], "w") as f:
                print("Saving matrix weight buffer: " +
                      os.path.relpath(buffers_export_info["matrix_weight_buffer"], export_info["root_dir"]))
                f.write(str(buffers.num_weights_of_matrices_per_vertex()) + "\n")
                f.write(str(buffers.num_elements()) + "\n")
                for elem_idx in range(buffers.num_elements()):
                    elem = buffers.element_at_index(elem_idx)
                    for weight_idx in range(elem.num_weights_of_matrices()):
                        f.write(float_to_string(elem.weight_of_matrix(weight_idx)) + "\n")

            buffers_export_info["matrix_index_buffer"] = os.path.join(skeletal_root_dir, "indices.txt")
            with open(buffers_export_info["matrix_index_buffer"], "w") as f:
                print("Saving matrix index buffer: " +
                      os.path.relpath(buffers_export_info["matrix_index_buffer"], export_info["root_dir"]))
                f.write(str(buffers.num_indices_of_matrices_per_vertex()) + " \n")
                f.write(str(buffers.num_elements()) + "\n")
                for elem_idx in range(buffers.num_elements()):
                    elem = buffers.element_at_index(elem_idx)
                    for index_idx in range(elem.num_indices_of_matrices()):
                        f.write(str(elem.index_of_matrix(index_idx)) + "\n")


def save_mesh_spatial_alignment_to_armature(
        obj,                    # An instance of 'bpy.types.Object' with "obj.data" being of type 'bpy.type.Mesh'.
        buffers_export_info,    # A dictionary holding properties related to the buffers export.
        root_dir                # The root dir of the whole export.
        ):
    buffers_export_info["alignment_to_skeleton"] = os.path.join(buffers_export_info["skeletal_root_dir"], "alignment.txt")
    with open(buffers_export_info["alignment_to_skeleton"], "w") as f:
        print("Saving alignment-to-skeleton coord. system: " +
              os.path.relpath(buffers_export_info["alignment_to_skeleton"], root_dir))
        for i in range(3):
            f.write(float_to_string(obj.location[i]) + "\n")
        orientation = get_object_rotation_quaternion(obj)
        for i in range(4):
            f.write(float_to_string(orientation[i]) + "\n")


def save_mesh_links_to_textures(
        mtl_index,              # Index of material for which the links should be generated.
        buffers_export_info,    # A dictionary holding properties related to the buffers export.
        export_info             # A dictionary holding properties related to the export.
        ):
    texture_links_output_dir = os.path.join(buffers_export_info["mesh_root_dir"], "textures")

    texture_file_name = ["diffuse", "specular", "normal"]
    if len(buffers_export_info["texcoord_buffers"]) > len(texture_file_name):
        print("WARNING: The mesh uses too many texture coordinates. So, only first " + str(len(texture_file_name)) +
              "will be used.")
    buffers_export_info["texture_links"] = []
    for tex_idx in range(len(texture_file_name)):
        key = (mtl_index, texture_file_name[tex_idx])
        if key not in export_info["textures"]:
            continue
        buffers_export_info["texture_links"].append(
            os.path.join(texture_links_output_dir, texture_file_name[tex_idx] + ".txt")
            )
        os.makedirs(texture_links_output_dir, exist_ok=True)
        with open(buffers_export_info["texture_links"][-1], "w") as f:
            print("Saving texture link: " +
                  os.path.relpath(buffers_export_info["texture_links"][-1], export_info["root_dir"]))
            f.write(str(tex_idx if tex_idx < len(buffers_export_info["texcoord_buffers"]) else 0) + "\n")
            f.write(os.path.relpath(export_info["textures"][key], export_info["root_dir"]) + "\n")


def save_textures(
        materials,      # A list of all materials used in an exported mesh
        export_info     # A dictionary holding properties related to the export.
        ):
    """
    Returns a map from indices of processed materials to a list of path-names of texture files used in the material.
    :param materials: A list of all materials used in an exported mesh.
    :param export_info: A dictionary holding properties related to the exported mesh. Both keys and values are strings.
    :return: None
    """

    with TimeProf.instance().start("save_textures"):
        textures_root_dir = os.path.join(export_info["root_dir"], "textures")

        export_info["textures"] = {}
        for mat_idx in range(len(materials)):
            if materials[mat_idx] is None:
                continue
            for slot_idx in range(len(materials[mat_idx].texture_slots)):
                slot = materials[mat_idx].texture_slots[slot_idx]
                if slot is None:
                    continue

                if slot.use_map_color_diffuse:
                    texture_kind_name = "diffuse"
                elif slot.use_map_color_spec:
                    texture_kind_name = "specular"
                elif slot.use_map_normal:
                    texture_kind_name = "normal"
                else:
                    print("WARNING: Unsupported texture kind in slot " + str(slot_idx) +
                          " in material " + str(mat_idx) + " called " + materials[mat_idx].name +
                          ". Skipping the texture.")
                    continue

                texture = slot.texture
                if not hasattr(texture, "image"):
                    continue
                image = texture.image

                texture_output_dir = os.path.join(
                    textures_root_dir,
                    remove_ignored_part_of_name(materials[mat_idx].name, "TEXTURES")
                    )

                os.makedirs(texture_output_dir, exist_ok=True)

                def export_image(img):
                    ximage = img.copy()
                    if len(ximage.filepath_raw) == 0:
                        ximage.filepath_raw = "E2_tmp_image.png"
                    ximage.pack(as_png=True)
                    ximage.unpack(method="USE_LOCAL")
                    pathname = bpy.path.abspath(ximage.filepath_raw)
                    bpy.data.images.remove(bpy.data.images[ximage.name])
                    return pathname

                src_image_pathname = export_image(image)  # Get the saved image disc location

                dst_image_name = os.path.splitext(os.path.basename(texture.name))[0]
                if len(dst_image_name) == 0:
                    dst_image_name = os.path.splitext(os.path.basename(src_image_pathname))[0]
                dst_image_extension = ".png"  # The final image is always PNG, see 'image.pack(as_png=True)' above
                dst_image_pathname = os.path.join(texture_output_dir, dst_image_name + dst_image_extension)

                print("Copying image in slot #" + str(slot_idx) + " of material " + materials[mat_idx].name + ": " +
                      os.path.relpath(dst_image_pathname, export_info["root_dir"]))
                shutil.copyfile(src_image_pathname, dst_image_pathname)

                if bpy.path.abspath(image.filepath) != src_image_pathname:
                    os.remove(src_image_pathname)

                dst_texture_pathname = os.path.join(texture_output_dir, dst_image_name + ".txt")
                with open(dst_texture_pathname, "w") as f:
                    print("Saving texture in slot #" + str(slot_idx) + " of material " + materials[mat_idx].name +
                          ": " + os.path.relpath(dst_texture_pathname, export_info["root_dir"]))
                    f.write("./" + dst_image_name + dst_image_extension + "\n")
                    f.write("RGB" + ("A" if image.use_alpha else "") + "\n")
                    f.write("REPEAT\n")
                    f.write("REPEAT\n")
                    f.write("LINEAR_MIPMAP_LINEAR\n")
                    f.write("LINEAR\n")
                export_info["textures"][(mat_idx, texture_kind_name)] = dst_texture_pathname


def save_coord_systems_of_bones(
        armature,       # An instance of "bpy.types.Armature"
        export_info     # A dictionary holding properties of the export. Both keys and values are strings.
        ):
    """
    This function exports coordinate systems of all bones of the passed armature. The function also updates
    'export_info' so that 'export_info["coord_systems"]' is the pathname of the exported file.
    :param armature: An instance of 'bpy.types.Armature'.
    :param export_info: A dictionary holding properties of the export.
    :return: None
    """
    with TimeProf.instance().start("save_coord_systems_of_bones"):
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

            # Finally we compute the position (origin) and rotation (orientation) of the
            # coordinate system of the bone.
            pos = transform_matrix.inverted() * mathutils.Vector((0.0, 0.0, 0.0, 1.0))
            pos.resize_3d()
            rot = transform_matrix.to_3x3().transposed().to_quaternion().normalized()

            # And we store the computed data (i.e. the coordinate system).
            coord_systems.append({ "position": pos, "orientation": rot })

        export_info["pose"] = os.path.join(
            export_info["root_dir"],
            "animations",
            "skeletal",
            remove_ignored_part_of_name(armature.name, "SKELETAL"),
            "pose.txt"
            )
        os.makedirs(os.path.dirname(export_info["pose"]), exist_ok=True)
        with open(export_info["pose"], "w") as f:
            print("Saving pose coordinate systems: " +
                  os.path.relpath(export_info["pose"], export_info["root_dir"]))
            f.write(str(len(coord_systems)) + "\n")
            for system in coord_systems:
                for i in range(3):
                    f.write(float_to_string(system["position"][i]) + "\n")
                for i in range(4):
                    f.write(float_to_string(system["orientation"][i]) + "\n")


def save_keyframe_coord_systems_of_bones(
        armature,       # An instance of "bpy.types.Armature".
        export_info     # A dictionary holding properties related to the export.
        ):
    """
    This function exports coordinate systems of all keyframes of an animation of the bones of the passed armature.
    The function also updates "export_info" so that 'export_info["keyframe_coord_systems"]' is a list of the pathnames
    of the exported files (one file per keyframe).
    :param obj: An instance of "bpy.types.Armature".
    :param export_info: A dictionary holding properties related to the export.
    :return: None
    """
    with TimeProf.instance().start("save_keyframe_coord_systems_of_bones"):
        coord_systems_of_frames = {}

        if not armature.animation_data:
            return
        if not armature.animation_data.action:
            return

        scene = bpy.context.scene

        keyframes = set()
        action = armature.animation_data.action
        for fcurve in action.fcurves:
            for point in fcurve.keyframe_points:
                keyframes.add(round(point.co[0]))
        start_frame = min(keyframes)

        frame_current_backup = scene.frame_current

        for frame in keyframes:
            scene.frame_set(frame)
            coord_systems = []
            for armature_bone in armature.data.bones:
                if not armature_bone.use_deform:
                    continue
                bone = armature.pose.bones[armature_bone.name]
                coord_systems.append({
                    "position": bone.matrix * mathutils.Vector((0.0, 0.0, 0.0, 1.0)),
                    "orientation": bone.matrix.to_quaternion()
                    })
            coord_systems_of_frames[(frame - start_frame) * 0.041666666] = coord_systems

        scene.frame_set(frame_current_backup)

        # It remains to save the computed coordinate systems of bones in individual frames to disc.
        # We store each frame into a separate file. But all files will be written into the same output directory:

        keyframes_output_dir = os.path.join(
            export_info["root_dir"],
            "animations",
            "skeletal",
            remove_ignored_part_of_name(armature.name, "SKELETAL"),
            remove_ignored_part_of_name(action.name, "SKELETAL")
            )
        os.makedirs(keyframes_output_dir, exist_ok=True)

        export_info["keyframes"] = []  # Here we store pathnames of all saved files.
        frame_idx = 0
        for time_stamp in sorted(coord_systems_of_frames.keys()):
            export_info["keyframes"].append(
                os.path.join(keyframes_output_dir, "keyframe" + str(frame_idx) + ".txt")
                )
            with open(export_info["keyframes"][-1], "w") as f:
                print("Saving keyframe " + str(frame_idx + 1) + "/" + str(len(coord_systems_of_frames)) + ": " +
                      os.path.relpath(export_info["keyframes"][-1], export_info["root_dir"]))
                f.write(float_to_string(time_stamp) + "\n")
                f.write(str(len(coord_systems_of_frames[time_stamp])) + "\n")
                for system in coord_systems_of_frames[time_stamp]:
                    for i in range(3):
                        f.write(float_to_string(system["position"][i]) + "\n")
                    for i in range(4):
                        f.write(float_to_string(system["orientation"][i]) + "\n")
            frame_idx += 1


def save_draw_state_file(export_info, buffers_index):
    buffers_export_info = export_info["render_buffers"][buffers_index]
    with open(os.path.join(buffers_export_info["mesh_root_dir"], "draw_state.txt"), "w") as ofile:
        ofile.write(
            "use_alpha_blending              false\n"
            "alpha_blending_src_function     SRC_ALPHA\n"
            "alpha_blending_dst_function     ONE_MINUS_CONSTANT_ALPHA\n"
            "cull_face_mode                  BACK\n"
            )


def save_effects_file(export_info, buffers_index):
    buffers_export_info = export_info["render_buffers"][buffers_index]
    with open(os.path.join(buffers_export_info["mesh_root_dir"], "effects.txt"), "w") as ofile:
        ofile.write(
            "use_alpha_testing               false\n"
            "alpha_test_constant             0.0\n"
            "lighting_algo_location          fragment_program\n"
            "fog_algo_location               vertex_program\n"
            )



def export_object_mesh(
        obj,            # A selected object with MESH data to be exported
        export_dir      # A directory under which all exported files will be saved.
        ):
    assert obj.type == "MESH"
    with TimeProf.instance().start("export_object_armature"):
        print("Exporting MESH: " + obj.name)

        mesh = obj.data
        mesh_name = remove_ignored_part_of_name(mesh.name, "BUFFERS")

        num_armatures = 0
        armature = None
        for mod in obj.modifiers:
            if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
                armature = mod.object
                num_armatures += 1

        with TimeProf.instance().start("export_object_armature [consistency checks]"):
            if mesh_name is None or len(mesh_name) == 0:
                print("ERROR: The mesh has invalid name.")
                return
            if not is_correct_scale_vector(obj.scale):
                print("ERROR: The mesh is scaled: " + str(obj.scale) + ".")
                return
            if not vertices_have_correct_weights(mesh.vertices):
                return
            if len(mesh.vertex_colors) > 2:
                print("ERROR: The mesh has more than 2 colours per vertex.")
                return

            if num_armatures > 1:
                print("ERROR: The mesh has more than one armature.")
                return
            if armature is not None and len(armature.data.bones) == 0:
                print("ERROR: The armature '" + armature.name + "' of the mesh has no bone.")
                return
            if armature is not None and not does_vertex_groups_names_match_names_of_bones(obj, armature):
                return

        export_info = {
            "root_dir": export_dir,
            "mesh_name": mesh_name
        }

        save_textures(mesh.materials, export_info)
        if len(mesh.uv_layers) > len(export_info["textures"]):
            print("ERROR: len(mesh.uv_layers) > texture slots in materials.")
            return

        buffers_list = build_render_buffers(obj, armature)
        assert len(buffers_list) <= 1 or len(buffers_list) <= len(mesh.materials)
        for idx in range(len(buffers_list)):
            if len(buffers_list) > 1:
                print("--- Exporting a batch for material '" + mesh.materials[idx].name + "' ---")
            save_render_buffers(
                buffers_list[idx],
                remove_ignored_part_of_name(mesh.materials[idx].name, "BUFFERS") if len(buffers_list) > 1 else None,
                remove_ignored_part_of_name(armature.name, "BUFFERS") if armature is not None else None,
                export_info
                )
            buffers_export_info = export_info["render_buffers"][-1]
            if armature is not None:
                save_mesh_spatial_alignment_to_armature(obj, buffers_export_info, export_info["root_dir"])
            if buffers_list[idx].num_texture_coords() > 0:
                save_mesh_links_to_textures(idx, buffers_export_info, export_info)

            save_draw_state_file(export_info, idx)
            save_effects_file(export_info, idx)


def export_object_armature(
        obj,            # A selected object with ARMATURE data to be exported
        export_dir      # A directory under which all exported files will be saved.
        ):
    assert obj.type == "ARMATURE"
    with TimeProf.instance().start("export_object_armature"):
        print("Exporting ARMATURE: " + obj.name)

        if len(obj.data.bones) == 0:
            print("ERROR: The armature has no bone.")
            return
        if not armature_has_correct_scales(obj):
            return

        export_info = {"root_dir": export_dir}

        save_coord_systems_of_bones(obj, export_info)
        save_keyframe_coord_systems_of_bones(obj, export_info)


def export_object(
        obj,            # The object to be exported (either MESH or ARMATURE)
        export_dir      # A directory under which all exported files will be saved.
        ):
    """
    This is the root function of the actual export. Its purpose is to check whether data to be exported satisfies
    requirement (like no-scale rule), and then manages the export process for the selected object.
    :param export_dir: A directory under which all exported files will be saved.
    :return: True or False indicating success or failure.
    """

    assert obj.type in ["MESH", "ARMATURE"]
    with TimeProf.instance().start("export_selected_object"):
        print("*** E2 gfx exporter started ***")
        if obj.type == "MESH":
            export_object_mesh(obj, export_dir)
        elif obj.type == "ARMATURE":
            export_object_armature(obj, export_dir)
        else:
            print("ERROR: Unsupported object type: " + str(obj.type) + " not in [MESH, ARMATURE]")
            return False
        print("*** E2 gfx exporter terminated ***")
        return True


class E2_gfx_exporter(bpy.types.Operator):
    """Save gfx data in E2 gfx format (hierarchy of directories and files; path-names define meaning of data)."""
    bl_idname = "export.e2_gfx_exporter"
    bl_label = "E2 gfx exporter"
    directory = bpy.props.StringProperty(
                    name="Output root directory",
                    description="A root directory under which all gfx data will be saved.",
                    subtype="DIR_PATH",
                    maxlen=1024,
                    default="")

    @classmethod
    def poll(cls, context):
        for obj in bpy.context.selected_objects:
            if obj.type not in ["MESH", "ARMATURE"]:
                return False
        return len(bpy.context.selected_objects) > 0

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def execute(self, context):
        try:
            for obj in bpy.context.selected_objects:
                if obj.type not in ["MESH", "ARMATURE"]:
                    print("ERROR: The object '" + obj.name + "' is of an unsupported type '" + str(obj.type) + "'.")
                    break
                if export_object(obj, os.path.normpath(self.directory)) is False:
                    break
        except Exception as e:
            print("EXCEPTION (unhandled): " + str(e))
            print(traceback.format_exc())
        return{'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(E2_gfx_exporter.bl_idname, text="E2 gfx exporter")


def register():
    bpy.utils.register_class(E2_gfx_exporter)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(E2_gfx_exporter)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()
