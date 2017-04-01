import bpy
import mathutils


def from_base_matrix(position,orientation):
    return mathutils.Matrix.Translation(position) * orientation.to_matrix().to_4x4()


def to_base_matrix(position,orientation):
    return orientation.to_matrix().to_4x4().transposed() * mathutils.Matrix.Translation(-position)


def is_correct_scale_vector(scale_vector):
    return abs(1.0 - scale_vector[0]) < 0.001 and\
           abs(1.0 - scale_vector[1]) < 0.001 and\
           abs(1.0 - scale_vector[2]) < 0.001


def does_selected_mesh_object_have_correct_scales(obj):
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
    


print("-------------------------------------------------")
for obj in bpy.context.selected_objects:
    if obj.type != 'MESH':
        print("  WARNING: Object '" + obj.name + "' is not a MESH. Skipping it...")
        continue
    if not does_selected_mesh_object_have_correct_scales(obj):
        continue
    for mod in obj.modifiers:
        if mod.type == 'ARMATURE' and mod.object.type == 'ARMATURE':
            armature = mod.object

            # This is the list into which we store coordinate systems of all bones in the armature
            # in the order as they are listed in the armature (to match their indices in vertex groups).                
            coord_systems = []

            # So, let's compute coordinate systems of individual bones.
            for bone in armature.data.bones:
                
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
                for xbone in bones_list:
                    origin = transform_matrix * xbone.head_local.to_4d()
                    origin.resize_3d()
                    transform_matrix = to_base_matrix(origin,xbone.matrix.to_quaternion()) * transform_matrix
                transform_matrix = transform_matrix * from_base_matrix(obj.location,obj.rotation_quaternion)
                
                # Finally we compute the position (origin) and rotation (orientation) of the
                # coordinate system of the bone.
                pos = transform_matrix.inverted() * mathutils.Vector((0.0, 0.0, 0.0, 1.0))
                pos.resize_3d()
                rot = transform_matrix.to_3x3().transposed().to_quaternion().normalized()
                
                # And we store the computed data (i.e. the coordinate system).
                coord_systems.append({ "position": pos, "orientation": rot })

            if False:
                # Let's print results of computed coordinate systems of bones
                for idx in range(0,len(coord_systems)):
                    print(armature.data.bones[idx].name)
                    system = coord_systems[idx]
                    print("  pos: " + str(system["position"]))
                    print("  rot: " + str(system["orientation"]))
                    print("  x-axis: " + str(system["orientation"].to_matrix() * mathutils.Vector((1.0,0.0,0.0))))
                    print("  y-axis: " + str(system["orientation"].to_matrix() * mathutils.Vector((0.0,1.0,0.0))))
                    print("  z-axis: " + str(system["orientation"].to_matrix() * mathutils.Vector((0.0,0.0,1.0))))
                    
            if False:
                # Debug print of coordinates of a vertex in the model space of the MESH object
                # transformed into the space of individual bones.
                print("Debug prints of coordinates of a chosen vertex:")
                vtx_model = obj.data.vertices[2].co
                print("  In model space: " + str(vtx_model))
                for idx in range(0,len(coord_systems)):
                    system = coord_systems[idx]
                    vtx_bone = to_base_matrix(system["position"],system["orientation"]) * vtx_model
                    print("  In space of " + armature.data.bones[idx].name + ": " + str(vtx_bone))
                    
            # Now we start computation of coordinate systems of the pose of the armature.
            # We store the coordinate systems of individual pose bones in the list below in the
            # order as the pose bones are listed in the armature to preserve the index-matching
            # with armature bones and vertex groups of vertices.
            pose_coord_systems = []
            
            # So, let's compute coordinate systems of individual pose bones.
            for pose_bone in armature.pose.bones:
                # We compute the "from-base" matrix of the pose bone by composing the "from-armature "
                # transformation matrix with the final "to-armature" matrix of the bone (i.e. the matrix
                # of the bone after constraints and drivers are all applied).
                transform_matrix = from_base_matrix(armature.location,armature.rotation_quaternion) *\
                                   pose_bone.matrix
                                   
                # Now we compute the position (origin) and rotation (orientation) of the
                # coordinate system of the pose bone.
                pos = transform_matrix * mathutils.Vector((0.0, 0.0, 0.0, 1.0))
                pos.resize_3d()
                rot = transform_matrix.to_3x3().to_quaternion().normalized()
                
                # And we store the computed data (i.e. the coordinate system).
                pose_coord_systems.append({ "position": pos, "orientation": rot })
                
            assert len(pose_coord_systems) == len(coord_systems)    

            if True:
                # Let's print results of computed coordinate systems of pose bones
                for idx in range(0,len(pose_coord_systems)):
                    print("pose " + armature.pose.bones[idx].name)
                    system = pose_coord_systems[idx]
                    print("  pos: " + str(system["position"]))
                    print("  rot: " + str(system["orientation"]))
                    print("  x-axis: " + str(system["orientation"].to_matrix() * mathutils.Vector((1.0,0.0,0.0))))
                    print("  y-axis: " + str(system["orientation"].to_matrix() * mathutils.Vector((0.0,1.0,0.0))))
                    print("  z-axis: " + str(system["orientation"].to_matrix() * mathutils.Vector((0.0,0.0,1.0))))
                    
            if True:
                # Debug print of coordinates of a vertex in the model space of the MESH object
                # transformed first to the space of a bone and then to the wold space from the
                # space of the corresponding pose bone.
                print("Debug prints of coordinates of a chosen vertex:")
                vtx_model = obj.data.vertices[8].co
                print("  In model space: " + str(vtx_model))
                for idx in range(0,len(coord_systems)):
                    system = coord_systems[idx]
                    pose_system = pose_coord_systems[idx]
                    vtx_world = from_base_matrix(pose_system["position"],pose_system["orientation"]) *\
                                to_base_matrix(system["position"],system["orientation"]) *\
                                vtx_model
                    print("  In world space: " + str(vtx_world))
