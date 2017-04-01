import bpy
import mathutils

print("-------------------------------------------------")
print(mathutils.Quaternion((1.0, 0.0, 0.0, 0.0)))
#for obj in bpy.data.objects:
#    print(obj)
#    for child in obj.children:
#        print(child)
#        for child2 in child.children:
#            print("* " + str(child2))
#bpy.ops.object.transform_apply(rotation=True, scale=True)
for obj in bpy.context.selected_objects:
    print("*" + obj.name + " :: " + str(obj.type) + " ~ " + str(obj))
    print("  pos: " + str(obj.location))
    print("  rot: " + str(obj.rotation_quaternion))
    for mod in obj.modifiers:
        if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
            #err print("    mod-loc: " + str(mod.location))
            #err print("    mod-mat: " + str(mod.matrix))
            armature = mod.object
            print("  *" + armature.name + " :: " + str(armature.type) + " ~ " + str(armature))
            print("    pos: " + str(armature.location))
            print("    rot: " + str(armature.rotation_quaternion))
            print("    children: " + str([x.name for x in armature.children]))
            print("    Bones of '" +  armature.data.name + "':")
            for bone in armature.data.bones:
                print("      *" + str(bone.name) + " :: " + str(bone))
                print("        head: " + str(bone.head))
                print("        tail: " + str(bone.tail))
                print("        head_local: " + str(bone.head_local))
                print("        tail_local: " + str(bone.tail_local))
                print("        vector: " + str(bone.vector))
                print("        center: " + str(bone.center))
                print("        x_axis: " + str(bone.x_axis))
                print("        y_axis: " + str(bone.y_axis))
                print("        z_axis: " + str(bone.z_axis))
                print("        use_local_location: " + str(bone.use_local_location))
                print("        matrix:\n" + str(bone.matrix))
                print("        matrix_local:\n" + str(bone.matrix_local))
                print("        parent: " + str(bone.parent))
                print("        children:    ")
                for ch in bone.children:
                    print("          " + ch.name)
                #print(bone.head * bone.matrix)
                #print(bone.tail * bone.matrix)
                #print("        " + str(bone.name) + " :: " + str(bone.tail) + "    " + str(bone.matrix))
            print("    Bones of pose " + str(armature.pose) + ":")
            for bone in armature.pose.bones:
                print("      *" + str(bone.name))
                print("        head: " + str(bone.head))
                print("        tail: " + str(bone.tail))
                print("        location: " + str(bone.location))
                print("        vector: " + str(bone.vector))
                print("        rotation_quaternion: " + str(bone.rotation_quaternion))
                print("        scale: " + str(bone.scale))
                print("        x_axis: " + str(bone.x_axis))
                print("        y_axis: " + str(bone.y_axis))
                print("        z_axis: " + str(bone.z_axis))
                print("        matrix:\n" + str(bone.matrix))  
                print("        matrix_basis:\n" + str(bone.matrix_basis))
                #print(bone.head * bone.matrix)
                #print(bone.tail * bone.matrix)
                #print("        " + str(bone.name) + " :: " + str(bone.tail) + "    " + str(bone.matrix))
            #print("      *vertex groups: " + str(len(armature.vertex_groups)))
            #for grp in armature.vertex_groups:
            #    print("        " + str(grp))
    """
    print("  *vertices:")
    mesh = obj.data
    for idx in range(0,len(mesh.vertices)):
        vtx = mesh.vertices[idx]
        print("    [" + str(idx) + "]:")
        print("      pos: " + str(vtx.co))
        print("      normal: " + str(vtx.normal))
        print("      groups:")
        for gidx in range(0,len(vtx.groups)):
            grp=vtx.groups[gidx]
            print("        [" + str(gidx) + "]:")
            print("          group: " + str(grp.group))
            print("          weight: " + str(grp.weight))        
    for grp in obj.vertex_groups:
        print("  *vertex group " + str(grp) + ":")
        print("    grp.index: " + str(grp.index))
        print("    grp.?: " + str(grp.index))
        print("    weight: " + str(grp.weight(3)))
    #print("  *vertex group: " + str(len(obj.vertex_groups)))
    """