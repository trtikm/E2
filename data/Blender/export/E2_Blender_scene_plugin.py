import bpy
import mathutils
import math
import os
import json


######################################################
# PROPERTIES OF SCENE OBJECTS
######################################################


class E2_UL_RequestInfoListItem(bpy.types.PropertyGroup):
    REQUEST_INFO_KIND=[
        # Python ident, UI name, description, UID
        ("INCREMENT_ENABLE_LEVEL_OF_TIMER", "INCREMENT_ENABLE_LEVEL_OF_TIMER", "Increment enable level of timer.", 1),
        ("DECREMENT_ENABLE_LEVEL_OF_TIMER", "DECREMENT_ENABLE_LEVEL_OF_TIMER", "Decrement enable level of timer.", 2),
        ("RESET_TIMER", "RESET_TIMER", "Reset timer's current time back to zero.", 3),
        ("INCREMENT_ENABLE_LEVEL_OF_SENSOR", "INCREMENT_ENABLE_LEVEL_OF_SENSOR", "Increment enable level of sensor.", 4),
        ("DECREMENT_ENABLE_LEVEL_OF_SENSOR", "DECREMENT_ENABLE_LEVEL_OF_SENSOR", "Decrement enable level of sensor.", 5),
        ("IMPORT_SCENE", "IMPORT_SCENE", "Import scene.", 6),
        ("ERASE_FOLDER", "ERASE_FOLDER", "Erase folder.", 7),
        ("SET_LINEAR_VELOCITY", "SET_LINEAR_VELOCITY", "Set linear velocity of a rigid body.", 8),
        ("SET_ANGULAR_VELOCITY", "SET_ANGULAR_VELOCITY", "Set angular velocity of a rigid body.", 9),
        ("UPDATE_RADIAL_FORCE_FIELD", "UPDATE_RADIAL_FORCE_FIELD", "Update accel of radial force field acting of a rigid body.", 10),
        ("UPDATE_LINEAR_FORCE_FIELD", "UPDATE_LINEAR_FORCE_FIELD", "Update accel of linear force field acting of a rigid body.", 11),
        ("LEAVE_FORCE_FIELD", "LEAVE_FORCE_FIELD", "Leave force field.", 12),
    ]
    kind: bpy.props.EnumProperty(
            name="Kind",
            description="Defines what kind of requst info this is.",
            items=REQUEST_INFO_KIND,
            default="INCREMENT_ENABLE_LEVEL_OF_SENSOR"
            )
            
    EVENT_TYPE=[
        ("TOUCHING", "TOUCHING", "Touching events of a sensor.", 1),
        ("TOUCH_BEGIN", "TOUCH_BEGIN", "Touch begin events of a sensor.", 2),
        ("TOUCH_END", "TOUCH_END", "Touch end events of a sensor.", 3),
        ("TIME_OUT", "TIME_OUT", "Time out of a timer.", 4),
    ]
    event: bpy.props.EnumProperty(
            name="Event",
            description="Defines what kind of event the request info corresponds to.",
            items=EVENT_TYPE,
            default="TOUCH_BEGIN"
            )


class E2_UL_RequestInfosList(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname):
        if self.layout_type in {'DEFAULT', 'COMPACT'}:
            layout.label(text="", translate=False, icon="DOT")
            layout.prop(item, "kind", text="", emboss=False, icon_value=icon)
            layout.prop(item, "event", text="", emboss=False, icon_value=icon)
        elif self.layout_type in {'GRID'}:
            layout.alignment = 'CENTER'
            layout.label(text="", icon_value=icon)


class E2_UL_RequestInfosListInsert(bpy.types.Operator):
    """Insert a new request info to the list."""
    
    bl_idname = "e2_ul_request_infos_list.insert"
    bl_label = "Insert"

    def execute(self, context):
        object_props = context.object.e2_custom_props
        object_props.request_info_items.add()
        object_props.request_info_index = len(object_props.request_info_items) - 1
        return{'FINISHED'}


class E2_UL_RequestInfosListErase(bpy.types.Operator):
    """Erase the selected request info from the list."""
    
    bl_idname = "e2_ul_request_infos_list.erase"
    bl_label = "Erase"

    def execute(self, context):
        object_props = context.object.e2_custom_props
        if len(object_props.request_info_items) > 0:
            object_props.request_info_items.remove(object_props.request_info_index)
            object_props.request_info_index = max(0, min(object_props.request_info_index,
                                                         len(object_props.request_info_items) - 1))
        return{'FINISHED'}


class E2_UL_RequestInfosListUp(bpy.types.Operator):
    """Moves the selected request info up in the list."""
    
    bl_idname = "e2_ul_request_infos_list.up"
    bl_label = "Up"

    def execute(self, context):
        object_props = context.object.e2_custom_props
        if len(object_props.request_info_items) > 0 and object_props.request_info_index > 0:
            object_props.request_info_items.move(object_props.request_info_index, object_props.request_info_index - 1)
            object_props.request_info_index = max(0, min(object_props.request_info_index - 1,
                                                         len(object_props.request_info_items) - 1))
        return{'FINISHED'}


class E2_UL_RequestInfosListDown(bpy.types.Operator):
    """Moves the selected request info down in the list."""
    
    bl_idname = "e2_ul_request_infos_list.down"
    bl_label = "Down"

    def execute(self, context):
        object_props = context.object.e2_custom_props
        if len(object_props.request_info_items) > 0 and object_props.request_info_index < len(object_props.request_info_items) - 1:
            object_props.request_info_items.move(object_props.request_info_index, object_props.request_info_index + 1)
            object_props.request_info_index = max(0, min(object_props.request_info_index + 1,
                                                         len(object_props.request_info_items) - 1))
        return{'FINISHED'}


class E2ObjectProps(bpy.types.PropertyGroup):
    OBJECT_KIND=[
        # Python ident, UI name, description, UID
        ("FOLDER", "FOLDER", "A folder.", 1),
        ("BATCH", "BATCH", "A render batch.", 2),
        ("COLLIDER", "COLLIDER", "A collider.", 3),
        # === Object kinds below are embedded to a FOLDER ===
        #("FRAME", "FRAME", "A frame.", 4),
        #("RIGID_BODY", "RIGID_BODY", "A rigid body.", 5),
        #("TIMER", "TIMER", "An timer.", 6),
        #("AGENT", "AGENT", "An agent.", 7),
        # === Object kinds below are embedded to a COLLIDER ===
        #("SENSOR", "Sensor", "A sensor.", 8),
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
    folder_defines_timer: bpy.props.BoolProperty(
            name="Add 'timer' to the folder.",
            description="Whether to add a timer record into the folder or not",
            default=False
            )
    
    #====================================================
    # BATCH PROPS
    
    BATCH_KIND=[
        # Python ident, UI name, description, UID
        ("GENERIC_BOX", "GENERIC_BOX", "A generic box.", 1),
        ("GENERIC_CAPSULE", "GENERIC_CAPSULE", "A generic capsule.", 2),
        ("GENERIC_SPHERE", "GENERIC_SPHERE", "A generic sphere.", 3),
        ("REGULAR_GFX_BATCH", "REGULAR_GFX_BATCH", "A regular gfx batch created by E2 model exporter.", 4),
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
        ("BOX", "BOX", "A box collider.", 1),
        ("CAPSULE", "CAPSULE", "A capsule collider.", 2),
        ("SPHERE", "SPHERE", "A sphere collider.", 3),
        ("TRIANGLE_MESH", "TRIANGLE_MESH", "A triangle mesh collider.", 4),
    ]
    collider_kind: bpy.props.EnumProperty(
            name="Collider kind",
            description="Defines what kind of collider this is.",
            items=COLLIDER_KIND,
            default="BOX"
            )

    COLLIDER_COLLISION_CLASS=[
        # Python ident, UI name, description, UID
        ("STATIC_OBJECT", "STATIC_OBJECT", "A static (not moveable) scene collider.", 1),
        ("COMMON_MOVEABLE_OBJECT", "COMMON_MOVEABLE_OBJECT", "A common moveable scene collider.", 2),
        ("HEAVY_MOVEABLE_OBJECT", "HEAVY_MOVEABLE_OBJECT", "An object so heavy that no force may affect its motion.", 3),
        ("AGENT_MOTION_OBJECT", "AGENT_MOTION_OBJECT", "An agent motion object, like roller.", 4),
        ("FIELD_AREA", "FIELD_AREA", "A collider representing the area where the field is acting.", 5),
        ("SENSOR_DEDICATED", "SENSOR_DEDICATED", "A sensor colliding with same kind of sensors and heavy objects.", 6),
        ("SENSOR_WIDE_RANGE", "SENSOR_WIDE_RANGE", "A sensor colliding also with common objects.", 7),
        ("SENSOR_NARROW_RANGE", "SENSOR_NARROW_RANGE", "A sensor colliding only with wide or narrow sensors.", 8),
        ("RAY_CAST_TARGET", "RAY_CAST_TARGET", "An obejct colliding only with ray casts.", 9),
    ]
    collider_collision_class: bpy.props.EnumProperty(
            name="Collision class",
            description="A collision class of the collider.",
            items=COLLIDER_COLLISION_CLASS,
            default="COMMON_MOVEABLE_OBJECT"
            )
    
    COLLIDER_MATERIAL_TYPE=[
        # Python ident, UI name, description, UID
        ("ASPHALT", "ASPHALT", "A collision material asphalt.", 1),
        ("CONCRETE", "CONCRETE", "A collision material concrete.", 2),
        ("DIRT", "DIRT", "A collision material dirt.", 3),
        ("GLASS", "GLASS", "A collision material glass.", 4),
        ("GRASS", "GRASS", "A collision material grass.", 5),
        ("GUM", "GUM", "A collision material gum.", 6),
        ("ICE", "ICE", "A collision material ice.", 7),
        ("LEATHER", "LEATHER", "A collision material leather.", 8),
        ("MUD", "MUD", "A collision material mud.", 9),
        ("PLASTIC", "PLASTIC", "A collision material plastic.", 10),
        ("RUBBER", "RUBBER", "A collision material rubber.", 11),
        ("STEEL", "STEEL", "A collision material steel.", 12),
        ("WOOD", "WOOD", "A collision material wood.", 13),
        ("NO_FRINCTION_NO_BOUNCING", "NO_FRINCTION_NO_BOUNCING", "A collision material 'no friction no bouncing'.", 14),
    ]
    collider_material_type: bpy.props.EnumProperty(
            name="Collision material",
            description="A collision material of the collider.",
            items=COLLIDER_MATERIAL_TYPE,
            default="CONCRETE"
            )

    collider_defines_sensor: bpy.props.BoolProperty(
            name="Attach 'sensor' to this collider.",
            description="Whether to attach a sensor record to this collider or not",
            default=False
            )

    #====================================================
    # RIGID BODY PROPS

    rigid_body_is_moveable: bpy.props.BoolProperty(
            name="Is moveable?",
            description="Whether the rigid body is moveable during simulation or not",
            default=True
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

    #====================================================
    # TIMER PROPS

    timer_period_in_seconds: bpy.props.FloatProperty(
            name="Period",
            description="Period of the timer in seconds.",
            default=1.0,
            min=0.001,
            max=10000.0,
            step=0.001
            )
    timer_target_enable_level: bpy.props.IntProperty(
            name="Enable level",
            description=("A value at which the timer becomes enabled"),
            default=1,
            min=1,
            max=100,
            step=1
            )
    timer_current_enable_level: bpy.props.IntProperty(
            name="Current level",
            description=("An initial value the enable level of the timer"),
            default=0,
            min=0,
            max=100,
            step=1
            )

    #====================================================
    # SENSOR PROPS

    sensor_target_enable_level: bpy.props.IntProperty(
            name="Enable level",
            description=("A value at which the sensor becomes enabled"),
            default=1,
            min=1,
            max=100,
            step=1
            )
    sensor_current_enable_level: bpy.props.IntProperty(
            name="Current level",
            description=("An initial value the enable level of the sensor"),
            default=0,
            min=0,
            max=100,
            step=1
            )

    #====================================================
    # REQUEST INFO PROPS

    request_info_items: bpy.props.CollectionProperty(type = E2_UL_RequestInfoListItem)
    request_info_index: bpy.props.IntProperty(name = "Index for E2_UL_RequestInfoListItem", default = 0)


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
        self.warn_root_object_is_not_folder(object, layout)
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

        row = layout.row()
        row.prop(object_props, "folder_defines_timer")
        if object_props.folder_defines_timer is True:
            self.draw_timer(layout.box(), object, object_props)

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
        self.warn_root_object_is_not_folder(object, layout)
        self.warn_parent_is_not_folder(object, layout)
        self.warn_has_children(object, layout)
        self.warn_origin_moved(object, layout)
        self.warn_no_frame_found(object.parent, layout)
        self.warn_wrong_shape_geometry(object, object_props.batch_kind, layout)

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
        self.warn_root_object_is_not_folder(object, layout)
        self.warn_parent_is_not_folder(object, layout)
        self.warn_has_children(object, layout)
        self.warn_origin_moved(object, layout)
        self.warn_no_frame_found(object.parent, layout)
        #self.warn_frame_already_has_collider(object.parent, layout)
        self.warn_wrong_shape_geometry(object, object_props.collider_kind, layout)

        row = layout.row()
        row.prop(object_props, "collider_kind")

        row = layout.row()
        if object.type == 'EMPTY':
            row.prop(object, "scale")
        else:
            row.prop(object, "dimensions")

        row = layout.row()
        print("\n\n" + object.name + str(object_props.collider_collision_class))
        row.prop(object_props, "collider_collision_class")

        row = layout.row()
        row.prop(object_props, "collider_material_type")
    
        row = layout.row()
        row.prop(object_props, "collider_defines_sensor")
        if object_props.collider_defines_sensor is True:
            self.draw_sensor(layout.box(), object, object_props)

    def draw_rigid_body(self, layout, object, object_props):
        self.warn_no_frame_found(object, layout)
        self.warn_rigid_body_in_parent_folder(object.parent, layout)
            
        row = layout.row()
        row.prop(object_props, "rigid_body_is_moveable")
        
        if object_props.rigid_body_is_moveable is True:
            row = layout.row()
            row.prop(object_props, "rigid_body_linear_velocity")
            
            row = layout.row()
            row.prop(object_props, "rigid_body_angular_velocity")

            row = layout.row()
            row.prop(object_props, "rigid_body_external_linear_acceleration")
            
            row = layout.row()
            row.prop(object_props, "rigid_body_external_angular_acceleration")

    def draw_timer(self, layout, object, object_props):
        row = layout.row()
        row.prop(object_props, "timer_period_in_seconds")
        row = layout.row()
        row.prop(object_props, "timer_target_enable_level")
        row = layout.row()
        row.prop(object_props, "timer_current_enable_level")
        self.draw_request_infos(layout, object, object_props, "TIMER")
        
    def draw_sensor(self, layout, object, object_props):
        row = layout.row()
        row.prop(object_props, "sensor_target_enable_level")
        row = layout.row()
        row.prop(object_props, "sensor_current_enable_level")
        self.draw_request_infos(layout, object, object_props, "SENSOR")

    def draw_request_infos(self, layout, object, object_props, kind):
        row = layout.row()
        row.label(text="Request infos:")
        row = layout.row()
        row.template_list("E2_UL_RequestInfosList", "RequestInfos", object_props, "request_info_items", object_props, "request_info_index")
        row = layout.row()
        row.operator("e2_ul_request_infos_list.insert", text="Insert")        
        row.operator("e2_ul_request_infos_list.up", text="Up")        
        row = layout.row()
        row.operator("e2_ul_request_infos_list.erase", text="Erase")        
        row.operator("e2_ul_request_infos_list.down", text="Down")        

    # == warnings ======================================================================

    def warn_root_object_is_not_folder(self, object, layout):        
        if object.parent is None and object.e2_custom_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! ERROR: An object without a parent can only be a folder !!!")

    def warn_parent_is_not_folder(self, object, layout):        
        if object.parent is not None and object.parent.e2_custom_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! ERROR: The direct parent object is not a folder !!!")

    def warn_folder_is_scaled(self, folder, layout):
        if any(abs(folder.scale[i] - 1.0) > 0.0001 for i in range(3)):
            row = layout.row()
            row.label(text="!!! ERROR: The folder is scaled !!!")

    def warn_has_children(self, object, layout):
        if len(object.children) > 0:
            row = layout.row()
            row.label(text="!!! ERROR: The " + object.e2_custom_props.object_kind + " has a child object !!!")

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
            
    def warn_wrong_shape_geometry(self, object, kind, layout):
        sizes = object.scale if object.type == 'EMPTY' else 0.5 * object.dimensions
        if "CAPSULE" in kind:
            if abs(sizes[1] - sizes[0]) > 0.0001:
                row = layout.row()
                row.label(text="!!! WARNING: The capsule is not scaled uniformly along axes x and y !!!")
            if sizes[2] - min(sizes[0], sizes[1]) < 0.0001:
                row = layout.row()
                row.label(text="!!! WARNING: The capsule is not scaled big enough along the z axis !!!")
        elif "SPHERE" in kind:
            if any(abs(sizes[i+1] - sizes[0]) > 0.0001 for i in range(2)):
                row = layout.row()
                row.label(text="!!! WARNING: The sphere is not scaled uniformly along all axes !!!")


######################################################
# SCENE IMPORT AND EXPORT
######################################################


def normalise_disk_path(path):
    return str(path).replace("\\", "/")


def num2str(num, precision=6):
    return format(num, "." + str(precision) + "f") if isinstance(num, float) else str(num)


def remove_numeric_suffix(name):
    idx = name.rfind(".")
    if idx >= 0 and idx + 1 < len(name) and all(c.isdigit() for c in name[idx+1:]):
        return name[:idx]
    return name


class E2SceneExportOperator(bpy.types.Operator):
    """ E2 scene exporter: Exports the scene under the export dir """
    bl_idname = "scene.e2_export"
    bl_label = "Export scene"
    bl_options = {'REGISTER', 'UNDO'}
    
    def execute(self, context):
        try:
            print("E2 Scene exporter: Exporting scene using to dir: " + context.scene.e2_scene_export_dir)
            self.export_scene(context.scene.objects, str(context.scene.e2_scene_export_dir),
                              context.scene.e2_scene_remove_name_numeric_suffixes_whenever_possible)
            print("E2 Scene exporter: Finished successfully.")
        except Exception as e:
            print("E2 Scene exporter: Export has FAILED. Details:\n" + str(e))
        return{'FINISHED'}

    def export_scene(self, objects, output_dir, remove_suffixes):
        if not os.path.isdir(output_dir):
            raise Exception("The export directory '" + output_dir + "' does not exist.")
        root_folders = []
        for object in objects:
            if object.parent is None and object.e2_custom_props.object_kind == "FOLDER":
                root_folders.append(object)
        result = { "folders":{}, "imports": {} }
        for folder in root_folders:
            if remove_suffixes is True:
                fixed_name = remove_numeric_suffix(folder.name)
                folder_name = fixed_name if fixed_name not in [n.name for n in root_folders] else folder.name
            else:
                folder_name = folder.name
            if len(folder.e2_custom_props.folder_imported_from_dir) > 0:
                result["imports"][folder_name] = normalise_disk_path(folder.e2_custom_props.folder_imported_from_dir)
            else:
                result["folders"][folder_name] = self.export_folder(folder, remove_suffixes)
        result = self.clean_result(result)    
        #print(json.dumps(result, indent=4, sort_keys=True))
        with open(os.path.join(output_dir, "hierarchy.json"), "w") as f:
            json.dump(result, f, indent=4, sort_keys=True)

    def export_folder(self, folder, remove_suffixes):
        result = { "content":{}, "folders":{}, "imports": {} }
        if folder.e2_custom_props.folder_defines_frame is True:
            result["content"]["FRAME"] = self.export_frame(folder)
        if folder.e2_custom_props.folder_defines_rigid_body is True:
            result["content"]["RIGID_BODY"] = self.export_rigid_body(folder)
        for child in folder.children:
            if remove_suffixes is True:
                fixed_name = remove_numeric_suffix(child.name)
                child_name = fixed_name if fixed_name not in [n.name for n in folder.children] else child.name
            else:
                child_name = child.name
            if child.e2_custom_props.object_kind == "FOLDER":
                if len(child.e2_custom_props.folder_imported_from_dir) > 0:
                    result["imports"][child_name] = normalise_disk_path(child.e2_custom_props.folder_imported_from_dir)
                else:
                    result["folders"][child_name] = self.export_folder(child, remove_suffixes)
            elif child.e2_custom_props.object_kind == "BATCH":
                result["content"][child_name] = self.export_batch(child)
            elif child.e2_custom_props.object_kind == "COLLIDER":
                result["content"][child_name] = self.export_collider(child)
                if child.e2_custom_props.collider_defines_sensor is True:
                    result["content"]["SENSOR." + child_name] = self.export_sensor(child, child_name)
        if folder.e2_custom_props.folder_defines_timer is True:
            result["content"]["TIMER"] = self.export_timer(folder)
        return self.clean_result(result)

    def export_frame(self, object):
        result = {
            "object_kind": "FRAME",
            "origin": {
                "x": num2str(object.location.x),
                "y": num2str(object.location.y),
                "z": num2str(object.location.z)
            },
            "orientation": {
                "x": num2str(object.rotation_quaternion.x),
                "y": num2str(object.rotation_quaternion.y),
                "z": num2str(object.rotation_quaternion.z),
                "w": num2str(object.rotation_quaternion.w)
            }
        }
        return result

    def export_batch(self, object):
        result = {
            "object_kind": object.e2_custom_props.object_kind,
            "batch_kind": object.e2_custom_props.batch_kind
        }
        if object.e2_custom_props.batch_kind == "REGULAR_GFX_BATCH":
            result["path"] = self.normalise_disk_path(object.e2_custom_props.batch_reguar_disk_path)
        else:
            sizes = object.scale if object.type == 'EMPTY' else 0.5 * object.dimensions
            if object.e2_custom_props.batch_kind == "GENERIC_BOX":
                result["half_sizes_along_axes"] = {
                    "x": num2str(sizes.x),
                    "y": num2str(sizes.y),
                    "z": num2str(sizes.z)
                }
            elif object.e2_custom_props.batch_kind == "GENERIC_CAPSULE":
                thickness = max(0.001, min(sizes.x, sizes.y))
                result["thickness_from_central_line"] = num2str(thickness)
                distance = max(0.002, sizes.z - thickness)
                result["half_distance_between_end_points"] = num2str(distance)
                result["num_lines_per_quarter_of_circle"] = num2str(
                    max(1, object.e2_custom_props.batch_generic_num_lines_per_quarter_of_circle)
                    )
            elif object.e2_custom_props.batch_kind == "GENERIC_SPHERE":
                result["radius"] = max(0.001, min(sizes))
                result["num_lines_per_quarter_of_circle"] = num2str(
                    max(1, object.e2_custom_props.batch_generic_num_lines_per_quarter_of_circle)
                    )
            try:
                material = object.data.materials[0].diffuse_color
                result["colour"] = {
                    "r": num2str(material[0]),
                    "g": num2str(material[1]),
                    "b": num2str(material[2]),
                    "a": num2str(material[3])
                }
            except Exception as e:
                result["colour"] = { "r": "0.75", "g": "0.75", "b": "1.0", "a": "1.0" }
        return result

    def export_collider(self, object):
        result = {
            "object_kind": object.e2_custom_props.object_kind,
            "collider_kind": object.e2_custom_props.collider_kind
        }
        if object.e2_custom_props.collider_kind == "TRIANGLE_MESH":
            pass
        else:
            sizes = object.scale if object.type == 'EMPTY' else 0.5 * object.dimensions
            if object.e2_custom_props.collider_kind == "BOX":
                result["half_sizes_along_axes"] = {
                    "x": num2str(sizes.x),
                    "y": num2str(sizes.y),
                    "z": num2str(sizes.z)
                }
            elif object.e2_custom_props.collider_kind == "CAPSULE":
                thickness = max(0.001, min(sizes.x, sizes.y))
                result["thickness_from_central_line"] = num2str(thickness)
                distance = max(0.002, sizes.z - thickness)
                result["half_distance_between_end_points"] = num2str(distance)
                result["num_lines_per_quarter_of_circle"] = num2str(
                    max(1, object.e2_custom_props.batch_generic_num_lines_per_quarter_of_circle)
                    )
            elif object.e2_custom_props.collider_kind == "SPHERE":
                result["radius"] = max(0.001, min(sizes))
                result["num_lines_per_quarter_of_circle"] = num2str(
                    max(1, object.e2_custom_props.batch_generic_num_lines_per_quarter_of_circle)
                    )
            result["collision_material"] = str(object.e2_custom_props.collider_collision_class)
            result["collision_class"] = str(object.e2_custom_props.collider_material_type)
        return result

    def export_rigid_body(self, object):
        object_props = object.e2_custom_props
        result = {
            "object_kind": "RIGID_BODY",
            "is_moveable": str(object_props.rigid_body_is_moveable),
        }
        if object_props.rigid_body_is_moveable is True:
            result["linear_velocity"] = {
                "x": num2str(object_props.rigid_body_linear_velocity[0]),
                "y": num2str(object_props.rigid_body_linear_velocity[1]),
                "z": num2str(object_props.rigid_body_linear_velocity[2])
                }
            result["angular_velocity"] = {
                "x": num2str(object_props.rigid_body_angular_velocity[0]),
                "y": num2str(object_props.rigid_body_angular_velocity[1]),
                "z": num2str(object_props.rigid_body_angular_velocity[2])
                }
            result["external_linear_acceleration"] = {
                "x": num2str(object_props.rigid_body_external_linear_acceleration[0]),
                "y": num2str(object_props.rigid_body_external_linear_acceleration[1]),
                "z": num2str(object_props.rigid_body_external_linear_acceleration[2])
                }
            result["external_angular_acceleration"] = {
                "x": num2str(object_props.rigid_body_external_angular_acceleration[0]),
                "y": num2str(object_props.rigid_body_external_angular_acceleration[1]),
                "z": num2str(object_props.rigid_body_external_angular_acceleration[2])
                }
        return result
    
    def export_timer(self, object):
        object_props = object.e2_custom_props
        result = {
            "object_kind": "TIMER",
            "period_in_seconds": str(object_props.timer_period_in_seconds),
            "target_enable_level": str(object_props.timer_target_enable_level),
            "current_enable_level": str(object_props.timer_current_enable_level)
        }
        return result
    
    def export_sensor(self, object, name):
        object_props = object.e2_custom_props
        result = {
            "object_kind": "SENSOR",
            "collider": name,
            "target_enable_level": str(object_props.sensor_target_enable_level),
            "current_enable_level": str(object_props.sensor_current_enable_level),
        }
        return result

    def clean_result(self, result):
        for key in ["content", "folders", "imports"]:
            if key in result and len(result[key]) == 0:
                del result[key]
        return result
    

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
        row.prop(scene, "e2_scene_remove_name_numeric_suffixes_whenever_possible")

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
    bpy.utils.register_class(E2_UL_RequestInfoListItem)
    bpy.utils.register_class(E2_UL_RequestInfosList)
    bpy.utils.register_class(E2_UL_RequestInfosListInsert)
    bpy.utils.register_class(E2_UL_RequestInfosListErase)
    bpy.utils.register_class(E2_UL_RequestInfosListUp)
    bpy.utils.register_class(E2_UL_RequestInfosListDown)
    bpy.utils.register_class(E2ObjectProps)
    bpy.utils.register_class(E2ObjectPropertiesPanel)
    bpy.utils.register_class(E2SceneExportOperator)
    bpy.utils.register_class(E2SceneImportOperator)
    bpy.utils.register_class(E2SceneIOPanel)
    bpy.types.Object.e2_custom_props = bpy.props.PointerProperty(type=E2ObjectProps)
    bpy.types.Scene.e2_scene_remove_name_numeric_suffixes_whenever_possible = bpy.props.BoolProperty(
            name="Try not to export numeric suffixes of names?",
            description=("During export try to ignore numeric suffixes\n"+
                         "of names, like '.001',  whenever possible."),
            default=True
            )
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
    del bpy.types.Scene.e2_scene_remove_name_numeric_suffixes_whenever_possible
    del bpy.types.Object.e2_custom_props
    bpy.utils.unregister_class(E2SceneIOPanel)
    bpy.utils.unregister_class(E2SceneImportOperator)
    bpy.utils.unregister_class(E2SceneExportOperator)
    bpy.utils.unregister_class(E2ObjectPropertiesPanel)
    bpy.utils.unregister_class(E2ObjectProps)
    bpy.utils.unregister_class(E2_UL_RequestInfosListDown)
    bpy.utils.unregister_class(E2_UL_RequestInfosListUp)
    bpy.utils.unregister_class(E2_UL_RequestInfosListErase)
    bpy.utils.unregister_class(E2_UL_RequestInfosListInsert)
    bpy.utils.unregister_class(E2_UL_RequestInfosList)
    bpy.utils.unregister_class(E2_UL_RequestInfoListItem)


if __name__ == "__main__":
    register()
    #unregister()
