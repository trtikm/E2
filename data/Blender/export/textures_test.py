import bpy
import os


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


def do_test(mesh):
    #print("   len(mesh.uv_layers) = " + str(len(mesh.uv_layers)))
    #for i in range(0,len(mesh.uv_textures)):
    #    print("   mesh.uv_textures[" + str(i) + "].name = " + str(mesh.uv_textures[i].name))
    #    #print("   mesh.uv_textures[" + str(i) + "].data.image = " + str(mesh.uv_textures[i].data.image))
    print("num_materials = " + str(len(mesh.materials)))
    for mtl_index in range(0,len(mesh.materials)):
        print("material[" + str(mtl_index) + "]:")
        images = get_texture_images_of_material(mesh.materials[mtl_index])
        print("   num_images = " + str(len(images)))
        for i in range(0,len(images)):
            image = images[i]
            print("   image[" + str(i) + "]:")
            print("      name = " + str(image.name))
            print("      filepath = " + str(image.filepath))
            print("      filepath_raw = " + str(image.filepath_raw))
            print("      file_format = " + str(image.file_format))
            print("      use_alpha = " + str(image.use_alpha))
            print("      alpha_mode = " + str(image.alpha_mode))
        #material = mesh.materials[mtl_index]
        #for mtex_slot_index in range(0,len(material.texture_slots)):
        #    mtex_slot = material.texture_slots[mtex_slot_index]
        #    if mtex_slot is not None:
        #        texture = mtex_slot.texture
        #        if hasattr(texture,"image"):
        #            image = texture.image
        #            print("mesh.materials[" + str(mtl_index) + "].texture_slots[" + str(mtex_slot_index) + "].texture.image.name = " + str(image.name))
        #            print("mesh.materials[" + str(mtl_index) + "].texture_slots[" + str(mtex_slot_index) + "].texture.image.filepath = " + str(image.filepath))
        #            print("mesh.materials[" + str(mtl_index) + "].texture_slots[" + str(mtex_slot_index) + "].texture.image.filepath_raw = " + str(image.filepath_raw))
                    #image.unpack(method="USE_LOCAL")#,id_name=image.name)

                    #old_path = image.filepath
                    #old_path_raw = image.filepath_raw
                    #new_path = "c:\\Users\\Marek\\root\\E2qtgl\\temp\\models\\palm_tree_simple\\palm_tree_simple\\aaa_" + str(mtl_index) + "_" + str(mtex_slot) + "_." + image.file_format
                    #image.filepath = new_path
                    #image.filepath_raw = new_path
                    #image.use_alpha = True
                    #image.alpha_mode = 'STRAIGHT'
                    #image.file_format = 'PNG'
                    #print("[updated] mesh.materials[" + str(mtl_index) + "].texture_slots[" + str(mtex_slot_index) + "].texture.image.filepath = " + str(image.filepath))
                    #bpy.ops.file.unpack_item(id_name=image.name,method="USE_ORIGINAL")
                    #image.filepath = old_path
                    #image.filepath_raw = old_path_raw
    #    print("   len(materials[" + str(i) + "].uv_textures) = " + str(len(mat.uv_textures)))



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
    print("\n\n\n***************************************************************")
    do_test(bpy.context.selected_objects[0].data)
else:
    print("ERROR: The script requires exacly one object of the 'MESH' type to be selected.")
