import bpy


######################################################
# PROPERTIES OF SCENE OBJECTS
######################################################


class E2ObjectProps(bpy.types.PropertyGroup):
    OBJECT_KIND=[
        # Python ident, UI name, description, UID
        ("FOLDER", "Folder", "A folder.", 1),
        ("BATCH", "Batch", "A render batch.", 2),
        ("COLLIDER", "Collider", "A collider.", 3),
        # === Object kinds below are embedded to a folder ===
        #("RIGID_BODY", "Rigid body", "A rigid body.", 4),
        #("SENSOR", "Sensor", "A sensor.", 5),
        #("ACTIVATOR", "Activator", "An activator.", 6),
        #("DEVICE", "Device", "A device.", 7),
        #("AGENT", "Agent", "An agent.", 8),
    ]
    object_kind: bpy.props.EnumProperty(
            name="Object kind",
            description="Defines what kind of object it represents.",
            items=OBJECT_KIND,
            default="FOLDER"
            )

    #====================================================
    # FOLDER PROPS
    
    folder_imported_from_dir: bpy.props.StringProperty(
            name="Imported from",
            description=("A directory from which the folder and all its conted\n"+
                         "were imported from. Empty string means not imported"),
            default="",
            maxlen=1000,
            subtype='NONE'
            )
    folder_defines_frame: bpy.props.BoolProperty(
            name="Add 'frame' to the folder.",
            description="Whether to add folder's coord system as a frame record\ninto the folder or not",
            default=True
            )
    folder_defines_rigid_body: bpy.props.BoolProperty(
            name="Add 'rigid body' to the folder.",
            description="Whether to add a rigid body record into the folder or not",
            default=False
            )
    
    #====================================================
    # BATCH PROPS
    
    BATCH_KIND=[
        # Python ident, UI name, description, UID
        ("GENERIC_BOX", "Generic box", "A generic box.", 1),
        ("GENERIC_CAPSULE", "Generic capsule", "A generic capsule.", 2),
        ("GENERIC_SPHERE", "Generic sphere", "A generic sphere.", 3),
        ("REGULAR_GFX_BATCH", "Regular gfx batch", "A regular gfx batch created by E2 model exporter.", 4),
    ]
    batch_kind: bpy.props.EnumProperty(
            name="Batch kind",
            description="Defines what kind of batch this is.",
            items=BATCH_KIND,
            default="GENERIC_BOX"
            )
    batch_generic_num_lines_per_quarter_of_circle: bpy.props.IntProperty(
            name="Num segments",
            description=("Number of line segments per quearter of rounded shape.\n"+
                         "Represents a level of details"),
            default=5,
            min=1,
            max=10,
            step=1
            )
    batch_reguar_disk_path: bpy.props.StringProperty(
            name="Batch file",
            description="A disk path to a file 'batch.txt' defining the batch",
            default="./batch.txt",
            maxlen=1000,
            subtype='FILE_PATH'
            )

    #====================================================
    # COLLIDER PROPS
    
    COLLIDER_KIND=[
        # Python ident, UI name, description, UID
        ("BOX", "Box", "A box collider.", 1),
        ("CAPSULE", "Capsule", "A capsule collider.", 2),
        ("SPHERE", "Sphere", "A sphere collider.", 3),
        ("TRIANGLE_MESH", "Triangle mesh", "A triangle mesh collider.", 4),
    ]
    collider_kind: bpy.props.EnumProperty(
            name="Collider kind",
            description="Defines what kind of collider this is.",
            items=COLLIDER_KIND,
            default="BOX"
            )

    COLLIDER_COLLISION_CLASS=[
        # Python ident, UI name, description, UID
        ("COMMON_SCENE_OBJECT", "Common scene object", "A common scene collider.", 1),
        ("INFINITE_MASS_OBJECT", "Infinite mass object", "An infinite mass object, like ground.", 2),
        ("AGENT_MOTION_OBJECT", "Agent motion object", "An agent motion object, like roller.", 3),
        ("SIGHT_TARGET", "Sight target", "A sight target.", 4),
        ("RAY_CAST_SIGHT", "Ray cast target", "A ray cast target", 5),
        ("SENSOR_GENERAL", "Sensor general", "A sensor colliding with activators and object colliders.", 6),
        ("SENSOR_SPECIAL", "Sensor special", "A sensor colliding only with activators.", 7),
        ("ACTIVATOR", "Activator", "An actoivator collider.", 8),
    ]
    collider_collision_class: bpy.props.EnumProperty(
            name="Collision class",
            description="A collision class of the collider.",
            items=COLLIDER_COLLISION_CLASS,
            default="COMMON_SCENE_OBJECT"
            )
    
    COLLIDER_MATERIAL_TYPE=[
        # Python ident, UI name, description, UID
        ("ASPHALT", "Asphalt", "A collision material asphalt.", 1),
        ("CONCRETE", "Concrete", "A collision material concrete.", 2),
        ("DIRT", "Dirt", "A collision material dirt.", 3),
        ("GLASS", "Glass", "A collision material glass.", 4),
        ("GRASS", "Grass", "A collision material grass.", 5),
        ("GUM", "Gum", "A collision material gum.", 6),
        ("ICE", "Ice", "A collision material ice.", 7),
        ("LEATHER", "Leather", "A collision material leather.", 8),
        ("MUD", "Mud", "A collision material mud.", 9),
        ("PLASTIC", "Plastic", "A collision material plastic.", 10),
        ("RUBBER", "Rubber", "A collision material rubber.", 11),
        ("STEEL", "Steel", "A collision material steel.", 12),
        ("WOOD", "Wood", "A collision material wood.", 13),
        ("NO_FRINCTION_NO_BOUNCING", "No friction no bouncing", "A collision material 'no friction no bouncing'.", 14),
    ]
    collider_material_type: bpy.props.EnumProperty(
            name="Collision material",
            description="A collision material of the collider.",
            items=COLLIDER_MATERIAL_TYPE,
            default="CONCRETE"
            )

    #====================================================
    # RIGID BODY PROPS

    rigid_body_is_moveable: bpy.props.BoolProperty(
            name="Is moveable?",
            description="Whether the rigid body is moveable during simulation or not",
            default=True
            )
    rigid_body_mass: bpy.props.FloatProperty(
            name="Mass",
            description="The mass of the rigid body. Value 0.0 means an infinite mass",
            default=1.0,
            unit='MASS',
            min=0.0,
            max=10000.0,
            step=0.001
            )
    rigid_body_compute_inverted_inertia_tesor_from_colliders: bpy.props.BoolProperty(
            name="Compute inverted inertia tensor from colliders?",
            description="Whether to compute the inverted inertia tensor from colliders or not",
            default=True
            )
    rigid_body_inverted_inertia_tensor_row_0: bpy.props.FloatVectorProperty(
            name="",
            description="Row 0 of the inverted inertia tensor",
            size=3,
            default=(1.0, 0.0, 0.0),
            unit='NONE',
            subtype='NONE',
            min=0.0,
            max=100.0,
            step=0.00001
            )
    rigid_body_inverted_inertia_tensor_row_1: bpy.props.FloatVectorProperty(
            name="",
            description="Row 1 of the inverted inertia tensor",
            size=3,
            default=(0.0, 1.0, 0.0),
            unit='NONE',
            subtype='NONE',
            min=0.0,
            max=100.0,
            step=0.00001
            )
    rigid_body_inverted_inertia_tensor_row_2: bpy.props.FloatVectorProperty(
            name="",
            description="Row 2 of the inverted inertia tensor",
            size=3,
            default=(0.0, 0.0, 1.0),
            unit='NONE',
            subtype='NONE',
            min=0.0,
            max=100.0,
            step=0.00001
            )
    rigid_body_linear_velocity: bpy.props.FloatVectorProperty(
            name="Linear velocity",
            description="An initial linear velocity of the rigid body",
            size=3,
            default=(0.0, 0.0, 0.0),
            unit='VELOCITY',
            subtype='VELOCITY',
            min=0.0,
            max=100.0,
            step=0.001
            )
    rigid_body_angular_velocity: bpy.props.FloatVectorProperty(
            name="Angular velocity",
            description="An initial angular velocity of the rigid body",
            size=3,
            default=(0.0, 0.0, 0.0),
            unit='VELOCITY',
            subtype='VELOCITY',
            min=0.0,
            max=100.0,
            step=0.001
            )
    rigid_body_external_linear_acceleration: bpy.props.FloatVectorProperty(
            name="Linear acceleration",
            description="An initial linear acceleration of the rigid body",
            size=3,
            default=(0.0, 0.0, 0.0),
            unit='ACCELERATION',
            subtype='ACCELERATION',
            min=0.0,
            max=100.0,
            step=0.001
            )
    rigid_body_external_angular_acceleration: bpy.props.FloatVectorProperty(
            name="Angular acceleration",
            description="An initial angular acceleration of the rigid body",
            size=3,
            default=(0.0, 0.0, 0.0),
            unit='ACCELERATION',
            subtype='ACCELERATION',
            min=0.0,
            max=100.0,
            step=0.001
            )


class E2ObjectPropertiesPanel(bpy.types.Panel):
    bl_idname = "OBJECT_PT_e2_object_props_panel"
    bl_label = "E2 object props"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = 'object'
    
    def draw(self, context):
        layout = self.layout
        object = context.object
        object_props = object.e2_custom_props

        row = layout.row()
        row.prop(object_props, "object_kind")
        if object_props.object_kind == "FOLDER":
            self.draw_folder(layout, object, object_props)
        elif object_props.object_kind == "BATCH":
            self.draw_batch(layout, object, object_props)
        elif object_props.object_kind == "COLLIDER":
            self.draw_collider(layout, object, object_props)

    def draw_folder(self, layout, object, object_props):
        self.warn_parent_is_not_folder(object, layout)
        self.warn_folder_is_scaled(object, layout)

        if len(object_props.folder_imported_from_dir) > 0:
            row = layout.row()
            row.label(text="Imported from: " + str(object_props.folder_imported_from_dir))

        row = layout.row()
        row.prop(object_props, "folder_defines_frame")
        if object_props.folder_defines_frame is True:
            self.draw_frame(layout.box(), object, object_props)

        row = layout.row()
        row.prop(object_props, "folder_defines_rigid_body")
        if object_props.folder_defines_rigid_body is True:
            self.draw_rigid_body(layout.box(), object, object_props)

    def draw_frame(self, layout, object, object_props):
        row = layout.row()
        row.prop(object, "location")

        row = layout.row()
        if object.rotation_mode == "AXIS_ANGLE":
            row.prop(object, "rotation_axis_angle")
        elif object.rotation_mode == "QUATERNION":
            row.prop(object, "rotation_quaternion")
        else:
            row.prop(object, "rotation_euler")
        
        row = layout.row()
        row.prop(object, "rotation_mode")    

    def draw_batch(self, layout, object, object_props):
        self.warn_parent_is_not_folder(object, layout)
        self.warn_origin_moved(object, layout)
        self.warn_no_frame_found(object.parent, layout)

        row = layout.row()
        row.prop(object_props, "batch_kind")
        if object_props.batch_kind == "REGULAR_GFX_BATCH":
            row = layout.row()
            row.prop(object_props, "batch_reguar_disk_path")
        else:
            row = layout.row()
            row.prop(object, "scale")
            row = layout.row()
            if object.type != 'EMPTY':
                row.prop(object, "dimensions")

            if object_props.batch_kind != "GENERIC_BOX":
                row = layout.row()
                row.prop(object_props, "batch_generic_num_lines_per_quarter_of_circle")

            row = layout.row()
            try:
                row.prop(object.data.materials[0], "diffuse_color")
            except Exception as e:
                row.label(text="!!! WARNING: Failed to find a material for the active object !!!")

    def draw_collider(self, layout, object, object_props):
        self.warn_parent_is_not_folder(object, layout)
        self.warn_origin_moved(object, layout)
        self.warn_no_frame_found(object.parent, layout)
        self.warn_frame_already_has_collider(object.parent, layout)

        row = layout.row()
        row.prop(object_props, "collider_kind")

        row = layout.row()
        if object.type == 'EMPTY':
            row.prop(object, "scale")
        else:
            row.prop(object, "dimensions")

        row = layout.row()
        row.prop(object_props, "collider_collision_class")

        row = layout.row()
        row.prop(object_props, "collider_material_type")
    
    def draw_rigid_body(self, layout, object, object_props):
        self.warn_no_frame_found(object, layout)
        self.warn_rigid_body_in_parent_folder(object.parent, layout)
            
        row = layout.row()
        row.prop(object_props, "rigid_body_is_moveable")
        
        if object_props.rigid_body_is_moveable is True:
            row = layout.row()
            row.prop(object_props, "rigid_body_mass")
            
            row = layout.row()
            row.prop(object_props, "rigid_body_compute_inverted_inertia_tesor_from_colliders")
            if object_props.rigid_body_compute_inverted_inertia_tesor_from_colliders is False:
                box = layout.box()
                row = box.row()
                row.label(text="Inverted inertia tesor:")
                row = box.row()
                row.prop(object_props, "rigid_body_inverted_inertia_tensor_row_0")
                row = box.row()
                row.prop(object_props, "rigid_body_inverted_inertia_tensor_row_1")
                row = box.row()
                row.prop(object_props, "rigid_body_inverted_inertia_tensor_row_2")
                
            row = layout.row()
            row.prop(object_props, "rigid_body_linear_velocity")
            
            row = layout.row()
            row.prop(object_props, "rigid_body_angular_velocity")

            row = layout.row()
            row.prop(object_props, "rigid_body_external_linear_acceleration")
            
            row = layout.row()
            row.prop(object_props, "rigid_body_external_angular_acceleration")

    def warn_parent_is_not_folder(self, object, layout):        
        if object.parent is not None and object.parent.e2_custom_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! ERROR: The direct parent object is not a folder !!!")

    def warn_folder_is_scaled(self, folder, layout):
        if any(abs(folder.scale[i] - 1.0) > 0.0001 for i in range(3)):
            row = layout.row()
            row.label(text="!!! ERROR: The folder is scaled !!!")

    def warn_origin_moved(self, object, layout):
        if any(abs(object.location[i]) > 0.0001 for i in range(3)):
            row = layout.row()
            row.label(text="!!! WARNING: The " + object.e2_custom_props.object_kind + " is moved from the origin !!!")

    def warn_no_frame_found(self, folder, layout):
        has_frame = False
        while folder != None:
            if folder.e2_custom_props.folder_defines_frame is True:
                has_frame = True
                break
            folder = folder.parent
        if has_frame is False:
            row = layout.row()
            row.label(text="!!! ERROR: Neither this nor any parent folder defines a frame !!!")

    def warn_frame_already_has_collider(self, folder, layout):
        collider_count = 0
        while folder != None:
            for folder_child in folder.children:
                if folder_child.e2_custom_props.object_kind == "COLLIDER":
                    collider_count += 1
            if folder.e2_custom_props.folder_defines_frame is True:
                break
            folder = folder.parent
        if collider_count > 1:
            row = layout.row()
            row.label(text="!!! WARNING: The closest parent folder with frame already has a collider !!!")

    def warn_rigid_body_in_parent_folder(self, folder, layout):
        while folder != None:
            if folder.e2_custom_props.folder_defines_rigid_body is True:
                row = layout.row()
                row.label(text="!!! ERROR: Some parent folder already defines a rigid body !!!")
                break
            folder = folder.parent


######################################################
# SCENE IMPORT AND EXPORT
######################################################


class E2SceneExportOperator(bpy.types.Operator):
    """ E2 scene exporter: Exports the scene under the export dir """
    bl_idname = "scene.e2_export"
    bl_label = "Export scene"
    bl_options = {'REGISTER', 'UNDO'}
    
    def export_scene(self, objects, output_dir):
        for ob in objects:
            print(ob)

    def execute(self, context):
        try:
            print("E2 Scene exporter: Exporting scene using to dir: " + context.scene.e2_scene_export_dir)
            self.export_scene(context.scene.objects, str(context.scene.e2_scene_export_dir))
            print("E2 Scene exporter: Finished successfully.")
        except Exception as e:
            print("E2 Scene exporter: Export has FAILED. Details:\n" + str(e))
        return{'FINISHED'}


class E2SceneImportOperator(bpy.types.Operator):
    """ E2 scene importer: Imports the scene from the import dir
        under the import folder """
    bl_idname = "scene.e2_import"
    bl_label = "Import scene"
    bl_options = {'REGISTER', 'UNDO'}

    def import_scene(self, objects, root_folder, input_dir):
        pass

    def execute(self, context):
        try:
            print("E2 Scene importer: Importing scene under '" + 
                  context.scene.e2_scene_import_under_folder +
                  "' from dir: " + context.scene.e2_scene_import_dir)
            self.import_scene(context.scene.objects,
                              str(context.scene.e2_scene_import_under_folder),
                              str(context.scene.e2_scene_export_dir))
            print("E2 Scene importer: Finished successfully.")
        except Exception as e:
            print("E2 Scene importer: Import has FAILED. Details:\n" + str(e))
        return{'FINISHED'}


class E2SceneIOPanel(bpy.types.Panel):
    bl_idname = "SCENE_PT_e2_scene_io_panel"
    bl_label = "E2 scene export and import"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = 'scene'
    
    def draw(self, context):
        layout = self.layout
        scene = context.scene
        
        row = layout.row()
        row.prop(scene, "e2_scene_export_dir")

        row = layout.row()
        row.operator("scene.e2_export")

        row = layout.row()
        row.prop(scene, "e2_scene_import_dir")

        row = layout.row()
        row.prop(scene, "e2_scene_import_under_folder")

        row = layout.row()
        row.operator("scene.e2_import")


######################################################
# REGISTRATION AND UNREGISTRATION TO AND FROM BLENDER
######################################################


def register():
    bpy.utils.register_class(E2ObjectProps)
    bpy.utils.register_class(E2ObjectPropertiesPanel)
    bpy.utils.register_class(E2SceneExportOperator)
    bpy.utils.register_class(E2SceneImportOperator)
    bpy.utils.register_class(E2SceneIOPanel)
    bpy.types.Object.e2_custom_props = bpy.props.PointerProperty(type=E2ObjectProps)
    bpy.types.Scene.e2_scene_export_dir = bpy.props.StringProperty(
            name="Export dir",
            description="A directory under which the scene will be exported",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
            )
    bpy.types.Scene.e2_scene_import_dir = bpy.props.StringProperty(
            name="Import dir",
            description="A directory from which a scene will be imported",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
            )
    bpy.types.Scene.e2_scene_import_under_folder = bpy.props.StringProperty(
            name="Import uder",
            description=("A scene in the current scene under which to import\n"+
                         "the scene. Empty string means the root folder"),
            default="",
            maxlen=1000,
            subtype='NONE'
            )

def unregister():
    del bpy.types.Scene.e2_scene_import_under_folder
    del bpy.types.Scene.e2_scene_import_dir
    del bpy.types.Scene.e2_scene_export_dir
    del bpy.types.Object.e2_custom_props
    bpy.utils.unregister_class(E2SceneIOPanel)
    bpy.utils.unregister_class(E2SceneImportOperator)
    bpy.utils.unregister_class(E2SceneExportOperator)
    bpy.utils.unregister_class(E2ObjectPropertiesPanel)
    bpy.utils.unregister_class(E2ObjectProps)


if __name__ == "__main__":
    register()
    #unregister()
