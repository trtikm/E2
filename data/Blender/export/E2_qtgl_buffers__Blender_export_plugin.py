bl_info = {
    "name": "E2::qtgl model export",
    "author": "E2",
    "version": (1, 0, 0),
    "blender": (2, 76, 0),
    "location": "File > Import-Export",
    "description": "E2::qtgl buffers export plugin",
    "warning": "",
    "wiki_url": "",
    "support": 'COMMUNITY',
    "category": 'Import-Export'
    }
    
import bpy
import os
import shutil

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

        
def  get_texture_images_of_material(material):
    images = []
    for mtex_slot_index in range(0,len(material.texture_slots)):
        mtex_slot = material.texture_slots[mtex_slot_index]
        if mtex_slot is not None:
            texture = mtex_slot.texture
            if hasattr(texture,"image"):
                image = texture.image
                images.append(image)
    return images


def save_buffers(
        buffers,    # An instance of the class 'render_buffers'.
        root_dir    # A directory into which individual output files will be created.
        ):
    precision_str = "%.6f"
        
    os.makedirs(root_dir, exist_ok=True)

    print("    Saving index buffer to '" + os.path.join(root_dir,"indices.txt") +"'.")
    f = open(os.path.join(root_dir,"indices.txt"),"w")
    f.write("E2::qtgl/buffer/indices/triangles/text\n")
    f.write(str(buffers.num_triangles()) + "\n")
    for i in range(0,buffers.num_triangles()):
        t = buffers.triangle_at_index(i)
        for j in range(0,3):
            assert t[j] >= 0 and t[j] < buffers.num_elements()
            f.write(str(t[j]) + "\n")
    f.close()
        
    print("    Saving vertex buffer to '" + os.path.join(root_dir,"vertices.txt") +"'.")
    f = open(os.path.join(root_dir,"vertices.txt"),"w")
    f.write("E2::qtgl/buffer/vertices/3d/text\n")
    f.write(str(buffers.num_elements()) + "\n")
    for i in range(0,buffers.num_elements()):
        c = buffers.element_at_index(i).vertex_coords()
        f.write((precision_str % c[0]) + "\n")
        f.write((precision_str % c[1]) + "\n")
        f.write((precision_str % c[2]) + "\n")
    f.close()

    print("    Saving normals to '" + os.path.join(root_dir,"normals.txt") +"'.")
    f = open(os.path.join(root_dir,"normals.txt"),"w")
    f.write("E2::qtgl/buffer/normals/3d/text\n")
    f.write(str(buffers.num_elements()) + "\n")
    for i in range(0,buffers.num_elements()):
        c = buffers.element_at_index(i).normal_coords()
        f.write((precision_str % c[0]) + "\n")
        f.write((precision_str % c[1]) + "\n")
        f.write((precision_str % c[2]) + "\n")
    f.close()

    print("    Saving diffuse colours to '" + os.path.join(root_dir,"diffuse_colours.txt") +"'.")
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
        
    print("    Saving specular colours to '" + os.path.join(root_dir,"specular_colours.txt") +"'.")
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
        print("    Saving coordinates of texture " + str(i) + "to '" +
              os.path.join(root_dir,"texcoords" + str(i) + ".txt") +"'.")
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
        root_dir    # A directory into which all exported buffer files will be created.
                    # In case more than one buffer must be generated, then several
                    # directories are created differing in a suffic "_N", where N
                    # is a number of generated batch.
        ):  # Returns a map from generated directories of individual buffers to indices
            # of corresponding materials.
    #print("export_mesh")
    #print("   root_dir = " + root_dir)
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
        #num_images = len(get_texture_images_of_material(mesh.materials[mtl_index]))
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
            #assert len(texcoords) <= num_images
            #while len(texcoords) < num_images:
            #    assert len(texcoords) > 0
            #    texcoords.append(texcoords[len(texcoords)-1])

            E = render_element(
                    mesh.vertices[mesh.loops[j].vertex_index].co,
                    mesh.vertices[mesh.loops[j].vertex_index].normal,
                    diffuse_colour,
                    specular_colour,
                    texcoords
                    )
            polygon.append(E)
        buffers[mtl_index].add_polygon(polygon)
        
    bufferdirs_to_mtlindices = {}
    if len(buffers) == 1:
        save_buffers(buffers[0],root_dir)
        bufferdirs_to_mtlindices[root_dir] = 0
    else:
        for i in range(0,len(buffers)):
            export_dir = root_dir + "_" + str(i)
            save_buffers(buffers[i],export_dir)
            bufferdirs_to_mtlindices[export_dir] = i
    return bufferdirs_to_mtlindices


def export_textures(
        materials,  # An list of all materials used in an exported mesh
        root_dir    # A directory into which all exported texture files will be created.
        ):  # Returns a map from indices of processed materials to a list of path-names
            # of texture files used in the material.
    os.makedirs(root_dir, exist_ok=True)
    #print("export_textures")
    #print("   root_dir = " + root_dir)
    #print("   num_materials = " + str(len(materials)))
    mtlindices_to_textures = {}
    for mtl_index in range(0,len(materials)):
        #print("      material[" + str(mtl_index) + "]:")
        material = materials[mtl_index]
        images = get_texture_images_of_material(materials[mtl_index])
        #print("      num_images = " + str(len(images)))
        for i in range(0,len(images)):
            image = images[i]
            #print("      image[" + str(i) + "]:")
            #print("         name = " + str(image.name))
            #print("         filepath = " + str(image.filepath))
            #print("         filepath_raw = " + str(image.filepath_raw))
            #print("         file_format = " + str(image.file_format))
            #print("         use_alpha = " + str(image.use_alpha))
            #print("         alpha_mode = " + str(image.alpha_mode))
            if os.path.exists(bpy.path.abspath(image.filepath)):
                src_image_pathname = bpy.path.abspath(image.filepath)
            else:
                image.unpack(method="USE_LOCAL")
                src_image_pathname = bpy.path.abspath("//textures/" + image.name)
                assert os.path.exists(src_image_pathname)
        
            dst_image_name = os.path.splitext(os.path.basename(src_image_pathname))[0]
            dst_image_extension = os.path.splitext(os.path.basename(src_image_pathname))[1]
            
            shutil.copyfile(src_image_pathname,os.path.join(root_dir,dst_image_name + dst_image_extension))
            
            texture_txt_file = os.path.join(root_dir,dst_image_name + ".txt")

            print("    Saving texture number " + str(i) + " to '" + texture_txt_file +"'.")
            f = open(texture_txt_file,"w")

            f.write("E2::qtgl/texture/text\n")
            f.write("./" + dst_image_name + dst_image_extension + "\n")
            f.write("COMPRESSED_RGBA\n")
            f.write("REPEAT\n")
            f.write("REPEAT\n")
            f.write("LINEAR_MIPMAP_LINEAR\n")
            f.write("LINEAR\n")
            
            f.close()

            if mtl_index not in mtlindices_to_textures:
                mtlindices_to_textures[mtl_index] = [ texture_txt_file ]
            else:
                mtlindices_to_textures[mtl_index].append(texture_txt_file)
                
    return mtlindices_to_textures


def export_batches(
        bufferdirs_to_mtlindices,   # A map from buffer directories to indices of corresponding materials.
        mtlindices_to_textures,     # A map from indices of materials to path-names of created texture files.
        num_uv_layers,              # A number of texture coordinates per a vertex, i.e. len(mesh.uv_layers)
        root_dir,                   # A directory into which all exported batch files will be created.
        ):  # Returns None.
    #print("export_batches")
    #print("   root_dir = " + root_dir)
    os.makedirs(root_dir, exist_ok=True)
    model_name = os.path.basename(root_dir)
    #print("   model_name = " + model_name)
    for buffers_dir, mtl_index in bufferdirs_to_mtlindices.items():
        batch_name = os.path.basename(buffers_dir)
        #print("   batch_name = " + batch_name)
        buffers_dir = os.path.join("../../meshes/",model_name,batch_name);
        #print("   buffers_dir = " + buffers_dir)
        if mtl_index in mtlindices_to_textures:
            image_files = mtlindices_to_textures[mtl_index]
        else:
            image_files = []
            
        print("    Saving render batch to '" + os.path.join(root_dir,batch_name + ".txt") +"'.")
        f = open(os.path.join(root_dir,batch_name + ".txt"),"w")

        f.write("E2::qtgl/batch/indexed/text\n")

        if len(image_files) == 0:
            f.write("../../shaders/vertex/vs_IpcUmOpcFc.a=1.txt\n")
            f.write("../../shaders/fragment/fs_IcFc.txt\n")
        elif len(image_files) == 1:
            f.write("../../shaders/vertex/vs_IptUmOptF.txt\n")
            f.write("../../shaders/fragment/fs_ItUdFd.txt\n")
        elif len(image_files) == 2:
            f.write("../../shaders/vertex/vs_Ipt2UmOptF.txt\n")
            f.write("../../shaders/fragment/fs_It2UdFdn.txt\n")
        else:
            f.write("../../shaders/vertex/??\n")
            f.write("../../shaders/fragment/??\n")

        f.write(os.path.join(buffers_dir,"indices.txt") + "\n")
        f.write("BINDING_IN_POSITION\n")
        f.write(os.path.join(buffers_dir,"vertices.txt") + "\n")
        for i in range(0,len(image_files)):
            f.write("BINDING_IN_TEXCOORD" + str(i) + "\n")
            if i < num_uv_layers:
                fname = "texcoords" + str(i) + ".txt"
            else:
                assert num_uv_layers > 0
                fname = "texcoords" + str(num_uv_layers - 1) + ".txt"            
            f.write(os.path.join(buffers_dir,fname) + "\n")

        if len(image_files) == 0:
            f.write("BINDING_IN_COLOUR\n")
            f.write(os.path.join(buffers_dir,"diffuse_colours.txt") + "\n")

        texture_binding_names = {
            0:"BINDING_TEXTURE_DIFFUSE",
            1:"BINDING_TEXTURE_NORMALS",
            }
        for i in range(0,len(image_files)):
            if i in texture_binding_names:
                f.write(texture_binding_names[i] + "\n")
            else:
                f.write("BINDING_TEXTURE_??\n")
            
            f.write(os.path.join("../../textures",model_name,os.path.basename(image_files[i])) + "\n")

        f.write("BACK\n")
        f.write("false\n")

        f.close()


def export_model(
        mesh,       # An instance of class 'bpy.types.Mesh'
        root_dir,   # A directory into which all exported files will be created.
        model_name, # A name of the exported model
        mesh_name   # A name of the current mesh
        ):
    #print("export_model")
    #print("   root_dir = " + root_dir)
    #print("   model_name = " + model_name)
    #print("   mesh_name = " + mesh_name)
    print("  Exporting '" + mesh.name + "' under '" + root_dir + "'.")
    bufferdirs_to_mtlindices = export_mesh(mesh,os.path.join(root_dir,"meshes",model_name,mesh_name))
    mtlindices_to_textures = export_textures(mesh.materials,os.path.join(root_dir,"textures",model_name))
    export_batches(bufferdirs_to_mtlindices,mtlindices_to_textures,len(mesh.uv_layers),os.path.join(root_dir,"models",model_name))


class E2_buffer_exporter(bpy.types.Operator):
    bl_idname = "export.e2_model"
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
        #return (len(bpy.context.selected_objects) == 1
        #        and bpy.context.selected_objects[0].type == "MESH")

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}
    
    def execute(self, context):
        print("Starting E2::qtgl model exporter.")

        bpy.ops.object.transform_apply(rotation=True, scale=True)

        export_dir = os.path.normpath(self.directory)
        
        model_name = os.path.basename(export_dir)
        if len(model_name) == 0:
            model_name = "model_name"
        mesh_names = set()

        for i in range(0,len(bpy.context.selected_objects)):
            obj = bpy.context.selected_objects[i]
            mesh = obj.data

            if mesh.name is None or len(mesh.name) == 0:
                mesh_name = "/batch_" + str(i)
            else:
                mesh_name = mesh.name
                if mesh_name in mesh_names:
                    mesh_name = mesh_name + str(i)
            mesh_names.add(mesh_name)

            export_model(mesh,export_dir,model_name,mesh_name)

        print("Terminating E2::qtgl model exporter.")
        return{'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(E2_buffer_exporter.bl_idname, text="E2 gtgl model")


def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)

if __name__ == "__main__":
    register()
