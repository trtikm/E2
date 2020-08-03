import bpy
import mathutils
import math
import os
import json


######################################################
# UTILITIES
######################################################


def normalise_disk_path(path, data_root_dir=None):
    path = os.path.abspath(str(path)).replace("\\", "/")
    if data_root_dir is not None:
        data_root_dir = os.path.abspath(str(data_root_dir)).replace("\\", "/")
        path = os.path.relpath(path, data_root_dir).replace("\\", "/")
    return path


def num2str(num, precision=6):
    return format(num, "." + str(precision) + "f") if isinstance(num, float) else str(num)


def bool2str(state, precision=6):
    return "true" if state is True else "false"


def list_of_name_of_scene_objects(self, context):
    return [(obj.name, obj.name, "", i) for i, obj in enumerate(context.collection.all_objects)]


def get_scene_object_of_name(object_name):
    return bpy.context.collection.all_objects[object_name]


def e2_custom_props_of(object_name):
    return get_scene_object_of_name(object_name).e2_custom_props


def is_root_folder(object):
    return object is not None and object.parent is None and object.name == "E2_ROOT"


def absolute_scene_path(object_name):
    path = []
    object = get_scene_object_of_name(object_name)
    while object is not None and object.name != "E2_ROOT":
        path.append(object.name)
        object = object.parent
    path.reverse()
    return "/".join(path)


def split_path(path):
    return [x for x in path.split("/") if len(x) > 0]


def relative_scene_path_from_absolute_paths(target_abs_path, start_abs_path):
    target = split_path(target_abs_path)
    start = split_path(start_abs_path)
    idx = min(len(target), len(start))
    for i in range(idx):
        if target[i] != start[i]:
            idx = i
            break
    rel_path = [".." for _ in range(len(start[idx:]))] + target[idx:]
    result = "/".join(rel_path) if len(rel_path) > 0 else "."
    return result


def relative_scene_path_to_folder(target_object_name, start_object_name):
    rel_path = relative_scene_path_from_absolute_paths(
                    absolute_scene_path(target_object_name),
                    absolute_scene_path(start_object_name)
                    )
    return rel_path + '/'
    

def relative_scene_path_to_folder_from_embedded(target_object_name, start_object_name):
    rel_path = relative_scene_path_to_folder(target_object_name, start_object_name)
    return "../" + ("" if rel_path == "./" else rel_path)


def relative_scene_path_to_content(target_object_name, start_object_name, record_name=None):
    rel_path = relative_scene_path_from_absolute_paths(
                    absolute_scene_path(target_object_name),
                    absolute_scene_path(start_object_name)
                    )
    if record_name is not None:
        rel_path += '/' + record_name
    return rel_path


def relative_scene_path_to_content_from_embedded(target_object_name, start_object_name, record_name=None):
    rel_path = relative_scene_path_to_content(target_object_name, start_object_name, record_name)
    if rel_path.startswith("./"):
        rel_path = '.' + rel_path
    else:
        rel_path = "../" + rel_path
    if record_name is not None and rel_path == "../" + record_name:
        return "."
    return rel_path


def relative_scene_path_to_embedded_content(target_object_name, start_object_name, record_name):
    rel_path = relative_scene_path_from_absolute_paths(
                    absolute_scene_path(target_object_name),
                    absolute_scene_path(start_object_name)
                    )
    if rel_path != '.':
        rel_path = "/".join(split_path(rel_path)[:-1]) + '/' + record_name
    return rel_path


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

    folder_of_timer: bpy.props.EnumProperty(
            name="Timer's folder",
            description="A folder of the considered timer.",
            items=list_of_name_of_scene_objects
            )

    collider_of_sensor: bpy.props.EnumProperty(
            name="Sensor's collider",
            description="A collider of the considered sensor.",
            items=list_of_name_of_scene_objects
            )

    import_dir: bpy.props.StringProperty(
            name="Import dir",
            description="A directory from which a scene will be imported",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
            )

    import_under_folder: bpy.props.EnumProperty(
            name="Under folder",
            description="A folder under which to import the scene.",
            items=list_of_name_of_scene_objects
            )

    import_relocation_frame_folder: bpy.props.EnumProperty(
            name="Relocation frame's folder",
            description="A folder containing a relocation frame for the imported scene.\n"
                        "A frame whose world space location will also be the location of\n"+
                        "imported root frames.",
            items=list_of_name_of_scene_objects
            )

    cache_imported_scene: bpy.props.BoolProperty(
            name="Cache imported scene.",
            description="Whether to cache the imported scene or not",
            default=True
            )

    import_motion_frame: bpy.props.EnumProperty(
            name="Motion frame's folder",
            description="A folder containing a motion frame for the imported scene.\n"+
                        "It is a local frame of the linear and angular velocity.",
            items=list_of_name_of_scene_objects
            )

    import_add_motion_frame_velocity: bpy.props.BoolProperty(
            name="Add motion frame velocity",
            description="Add velocity of the motion frame to imported rigid bodies",
            default=True
            )

    apply_linear_velocity: bpy.props.BoolProperty(
            name="Apply linear velocity",
            description="Apply linear velocity to imported rigid bodies",
            default=True
            )

    apply_angular_velocity: bpy.props.BoolProperty(
            name="Apply angular velocity",
            description="Apply angular velocity to imported rigid bodies",
            default=True
            )

    linear_velocity: bpy.props.FloatVectorProperty(
            name="Linear velocity",
            description="A linear velocity for the rigid body",
            size=3,
            default=(0.0, 0.0, 0.0),
            unit='VELOCITY',
            subtype='VELOCITY',
            min=-100.0,
            max=100.0,
            step=0.001
            )
    angular_velocity: bpy.props.FloatVectorProperty(
            name="Angular velocity",
            description="An angular velocity for the rigid body",
            size=3,
            default=(0.0, 0.0, 0.0),
            unit='VELOCITY',
            subtype='VELOCITY',
            min=-100.0,
            max=100.0,
            step=0.001
            )

    erase_folder: bpy.props.EnumProperty(
            name="Folder",
            description="A folder to be erased with all its content.",
            items=list_of_name_of_scene_objects
            )

    folder_of_rigid_body: bpy.props.EnumProperty(
            name="Rigid body's folder",
            description="A folder of the considered rigid body.",
            items=list_of_name_of_scene_objects
            )

    radial_force_field_multiplier: bpy.props.FloatProperty(
            name="Multiplier",
            description="The multiplier of the distance from the origin.",
            default=1.0,
            min=0.001,
            max=10000.0,
            step=0.001
            )

    radial_force_field_exponent: bpy.props.FloatProperty(
            name="Exponent",
            description="The exponent of the distance from the origin.",
            default=1.0,
            min=1.0,
            max=10000.0,
            step=0.001
            )

    radial_force_field_min_radius: bpy.props.FloatProperty(
            name="Minimal radius",
            description="The minimal radius in which the field is still acting.",
            default=0.001,
            min=0.001,
            max=10000.0,
            step=0.001
            )

    use_mass: bpy.props.BoolProperty(
            name="Use mass?",
            description="Whether to use mass of the rigid body or not in the\n"+
                        "computation of the acceleration acting on that body.",
            default=True
            )

    linear_force_field_acceleration: bpy.props.FloatVectorProperty(
            name="Acceleration",
            description="A linear acceleration acting in the linear field",
            size=3,
            default=(0.0, 0.0, -9.81),
            unit='ACCELERATION',
            subtype='ACCELERATION',
            min=-100.0,
            max=100.0,
            step=0.001
            )


class E2_UL_RequestInfosList(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname):
        if self.layout_type in {'DEFAULT', 'COMPACT'}:
            layout.label(text="", translate=False, icon="DOT")
            layout.prop(item, "kind", text="", emboss=False, icon_value=icon)
            layout.prop(item, "event", text="", emboss=False, icon_value=icon)
        elif self.layout_type in {'GRID'}:
            layout.alignment = 'CENTER'
            layout.label(text="", translate=False, icon="DOT")


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
                         "were imported from. Empty path means not imported"),
            default="",
            maxlen=1000,
            subtype='DIR_PATH'
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

    collider_triangle_mesh_dir: bpy.props.StringProperty(
            name="Mesh dir",
            description="A directory containing files defining a triangle mesh",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
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
            min=-100.0,
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
            min=-100.0,
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
            min=-100.0,
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
            min=-100.0,
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

    sensor_use_exclusive_trigger_collider: bpy.props.BoolProperty(
            name="Trigger sensor only by a concrete collider?",
            description="Whether to trigger sensor only by a concrete collider or not",
            default=False
            )
    sensor_trigger_collider: bpy.props.EnumProperty(
            name="Trigger collider",
            description="A concrete and only collider which will trigger the sensor.",
            items=list_of_name_of_scene_objects
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

        self.warn_not_under_root_folder(object, layout)
        self.warn_object_name_is_reserved(object.name, layout)
        self.warn_object_name_contains_reserved_character(object.name, layout)

        row = layout.row()
        row.prop(object_props, "object_kind")
        if object_props.object_kind == "FOLDER":
            self.draw_folder(layout, object, object_props)
        elif object_props.object_kind == "BATCH":
            self.draw_batch(layout, object, object_props)
        elif object_props.object_kind == "COLLIDER":
            self.draw_collider(layout, object, object_props)

    def draw_folder(self, layout, object, object_props):
        self.warn_root_folder_has_non_folder_content(object, object_props, layout)
        self.warn_root_folder_is_relocated(object, layout)
        self.warn_root_object_is_not_folder(object, layout)
        self.warn_parent_is_not_folder(object, layout)
        self.warn_object_is_scaled(object, layout, "folder")

        row = layout.row()
        row.prop(object_props, "folder_imported_from_dir")
        if len(object_props.folder_imported_from_dir) > 0:
            return

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
        self.warn_incomatible_collsion_class_with_rigid_body_moveable_flag(object, layout)
        self.warn_wrong_shape_geometry(object, object_props.collider_kind, layout)
        if object_props.collider_kind == "TRIANGLE_MESH":
            self.warn_object_is_scaled(object, layout, "Triangle mesh")
            self.warn_not_valid_triangle_mesh_dir(object_props.collider_triangle_mesh_dir, layout, "Mesh dir")

        row = layout.row()
        row.prop(object_props, "collider_kind")

        row = layout.row()
        if object_props.collider_kind == "TRIANGLE_MESH":
            row.prop(object_props, "collider_triangle_mesh_dir")
        else:
            if object.type == 'EMPTY':
                row.prop(object, "scale")
            else:
                row.prop(object, "dimensions")

        row = layout.row()
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
        box = layout.box()
        row = box.row()
        row.prop(object_props, "sensor_use_exclusive_trigger_collider")
        if object_props.sensor_use_exclusive_trigger_collider is True:
            self.warn_object_is_not_collider(object_props.sensor_trigger_collider, box, "Trigger collider")
            row = box.row()
            row.prop(object_props, "sensor_trigger_collider")
        self.draw_request_infos(layout, object, object_props, "SENSOR")

    def draw_request_infos(self, layout, object, object_props, kind):
        self.warn_request_info_wrong_event_type(object_props, kind, layout)
            
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
        if len(object_props.request_info_items) > 0:
            request_info = object_props.request_info_items[object_props.request_info_index]
            if request_info.kind == "INCREMENT_ENABLE_LEVEL_OF_TIMER":
                self.draw_request_info_increment_enable_level_of_timer(layout.box(), object_props, request_info)
            elif request_info.kind == "DECREMENT_ENABLE_LEVEL_OF_TIMER":
                self.draw_request_info_decrement_enable_level_of_timer(layout.box(), object_props, request_info)
            elif request_info.kind == "RESET_TIMER":
                self.draw_request_info_reset_timer(layout, object_props, request_info)
            elif request_info.kind == "INCREMENT_ENABLE_LEVEL_OF_SENSOR":
                self.draw_request_info_increment_enable_level_of_sensor(layout, object_props, request_info)
            elif request_info.kind == "DECREMENT_ENABLE_LEVEL_OF_SENSOR":
                self.draw_request_info_decrement_enable_level_of_sensor(layout, object_props, request_info)
            elif request_info.kind == "IMPORT_SCENE":
                self.draw_request_info_import_scene(layout, object, object_props, request_info)
            elif request_info.kind == "ERASE_FOLDER":
                self.draw_request_info_erase_folder(layout, object, object_props, request_info)
            elif request_info.kind == "SET_LINEAR_VELOCITY":
                self.draw_request_info_set_linear_velocity(layout, object_props, request_info)
            elif request_info.kind == "SET_ANGULAR_VELOCITY":
                self.draw_request_info_set_angular_velocity(layout, object_props, request_info)
            elif request_info.kind == "UPDATE_RADIAL_FORCE_FIELD":
                self.draw_request_info_update_radial_force_field(layout, object, object_props, request_info)
            elif request_info.kind == "UPDATE_LINEAR_FORCE_FIELD":
                self.draw_request_info_update_linear_force_field(layout, object, object_props, request_info)
            elif request_info.kind == "LEAVE_FORCE_FIELD":
                self.warn_object_is_not_collider_with_sensor(object.name, layout, "This object")
            else:
                raise Exception("ERROR: Unknown request info kind.")

    def draw_request_info_increment_enable_level_of_timer(self, layout, object_props, request_info):
        self.warn_object_is_not_folder_with_timer(request_info.folder_of_timer, layout, "Timer's folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.folder_of_timer, layout, "Timer's folder")
        row = layout.row()
        row.prop(request_info, "folder_of_timer")

    def draw_request_info_decrement_enable_level_of_timer(self, layout, object_props, request_info):
        self.warn_object_is_not_folder_with_timer(request_info.folder_of_timer, layout, "Timer's folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.folder_of_timer, layout, "Timer's folder")
        row = layout.row()
        row.prop(request_info, "folder_of_timer")

    def draw_request_info_reset_timer(self, layout, object_props, request_info):
        self.warn_object_is_not_folder_with_timer(request_info.folder_of_timer, layout, "Timer's folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.folder_of_timer, layout, "Timer's folder")
        row = layout.row()
        row.prop(request_info, "folder_of_timer")

    def draw_request_info_increment_enable_level_of_sensor(self, layout, object_props, request_info):
        self.warn_object_is_not_collider_with_sensor(request_info.collider_of_sensor, layout, "Sensor's collider")
        self.warn_object_of_name_is_not_under_root_folder(request_info.collider_of_sensor, layout, "Sensor's collider")
        row = layout.row()
        row.prop(request_info, "collider_of_sensor")

    def draw_request_info_decrement_enable_level_of_sensor(self, layout, object_props, request_info):
        self.warn_object_is_not_collider_with_sensor(request_info.collider_of_sensor, layout, "Sensor's folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.collider_of_sensor, layout, "Sensor's collider")
        row = layout.row()
        row.prop(request_info, "collider_of_sensor")

    def draw_request_info_import_scene(self, layout, object, object_props, request_info):
        self.warn_not_valid_import_dir(request_info.import_dir, layout, "Import dir")
        self.warn_object_is_not_folder(request_info.import_under_folder, layout, "Under folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.import_under_folder, layout, "Under folder")
        self.warn_object_is_not_folder_with_frame(request_info.import_relocation_frame_folder, layout, "Relocation frame", True)
        self.warn_object_of_name_is_not_under_root_folder(request_info.import_relocation_frame_folder, layout, "Relocation frame")
        if request_info.apply_linear_velocity is True or request_info.apply_angular_velocity is True:
            self.warn_object_is_not_folder_with_frame(request_info.import_motion_frame, layout, "Motion frame", True)
            self.warn_object_of_name_is_not_under_root_folder(request_info.import_motion_frame, layout, "Motion frame")

        row = layout.row()
        row.prop(request_info, "import_dir")
        row = layout.row()
        row.prop(request_info, "import_under_folder")
        row = layout.row()
        row.prop(request_info, "import_relocation_frame_folder")
        row = layout.row()
        row.prop(request_info, "cache_imported_scene")

        box = layout.box()
        row = box.row()
        row.prop(request_info, "apply_linear_velocity")
        if request_info.apply_linear_velocity is True:
            row = box.row()
            row.prop(request_info, "linear_velocity")

        box = layout.box()
        row = box.row()
        row.prop(request_info, "apply_angular_velocity")
        if request_info.apply_angular_velocity is True:
            row = layout.row()
            row.prop(request_info, "angular_velocity")

        if request_info.apply_linear_velocity is True or request_info.apply_angular_velocity is True:
            box = layout.box()
            row = box.row()
            row.prop(request_info, "import_motion_frame")
            if request_info.import_add_motion_frame_velocity is True:
                self.warn_not_under_rigid_body(object, box, "Motion frame's folder")
            row = box.row()
            row.prop(request_info, "import_add_motion_frame_velocity")

    def draw_request_info_erase_folder(self, layout, object, object_props, request_info):
        self.warn_object_is_not_folder(request_info.erase_folder, layout, "Folder to erase")
        self.warn_object_of_name_is_not_under_root_folder(request_info.erase_folder, layout, "Folder to erase")
        self.warn_is_root_folder(request_info.erase_folder, layout, "Folder to erase")
        row = layout.row()
        row.prop(request_info, "erase_folder")

    def draw_request_info_set_linear_velocity(self, layout, object_props, request_info):
        self.warn_folder_does_not_have_rigid_body(request_info.folder_of_rigid_body, layout, "Rigid body's folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.folder_of_rigid_body, layout, "Rigid body's folder")
        row = layout.row()
        row.prop(request_info, "folder_of_rigid_body")
        row = layout.row()
        row.prop(request_info, "linear_velocity")

    def draw_request_info_set_angular_velocity(self, layout, object_props, request_info):
        self.warn_folder_does_not_have_rigid_body(request_info.folder_of_rigid_body, layout, "Rigid body's folder")
        self.warn_object_of_name_is_not_under_root_folder(request_info.folder_of_rigid_body, layout, "Rigid body's folder")
        row = layout.row()
        row.prop(request_info, "folder_of_rigid_body")
        row = layout.row()
        row.prop(request_info, "angular_velocity")

    def draw_request_info_update_radial_force_field(self, layout, object, object_props, request_info):
        self.warn_object_is_not_collider_with_sensor(object.name, layout, "This object")
        row = layout.row()
        row.prop(request_info, "radial_force_field_multiplier")
        row = layout.row()
        row.prop(request_info, "radial_force_field_exponent")
        row = layout.row()
        row.prop(request_info, "radial_force_field_min_radius")
        row = layout.row()
        row.prop(request_info, "use_mass")

    def draw_request_info_update_linear_force_field(self, layout, object, object_props, request_info):
        self.warn_object_is_not_collider_with_sensor(object.name, layout, "This object")
        row = layout.row()
        row.prop(request_info, "linear_force_field_acceleration")
        row = layout.row()
        row.prop(request_info, "use_mass")

    # == warnings ======================================================================

    def warn_not_under_root_folder(self, object, layout, property_name=None):
        while object.parent is not None:
            object = object.parent
        if  object.name != "E2_ROOT":
            property_name = "object" if property_name is None else property_name
            row = layout.row()
            row.label(text="!!! WARNING: The " + property_name + " is not in the sub-tree the 'E2_ROOT' folder !!!")

    def warn_object_of_name_is_not_under_root_folder(self, object_name, layout, property_name):
        return self.warn_not_under_root_folder(get_scene_object_of_name(object_name), layout, property_name)

    def warn_root_folder_has_non_folder_content(self, object, object_props, layout):
        if  object.name == "E2_ROOT":
            if object_props.folder_defines_frame is True:
                row = layout.row()
                row.label(text="!!! WARNING: The 'E2_ROOT' folder defines a frame !!!")
            if object_props.folder_defines_rigid_body is True:
                row = layout.row()
                row.label(text="!!! WARNING: The 'E2_ROOT' folder defines a rigid body !!!")
            if object_props.folder_defines_timer is True:
                row = layout.row()
                row.label(text="!!! WARNING: The 'E2_ROOT' folder defines a timer !!!")        

    def warn_root_folder_is_relocated(self, object, layout):
        if  object.name == "E2_ROOT":
            self.warn_origin_moved(object, layout, 'E2_ROOT')
            self.warn_object_rotated(object, layout, 'E2_ROOT')
            self.warn_object_is_scaled(object, layout, "folder")

    def warn_root_object_is_not_folder(self, object, layout):        
        if object.parent is None and object.e2_custom_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! WARNING: An object without a parent can only be a folder !!!")

    def warn_parent_is_not_folder(self, object, layout):        
        if object.parent is not None and object.parent.e2_custom_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! WARNING: The direct parent object is not a folder !!!")

    def warn_origin_moved(self, object, layout, property_name=None):
        if any(abs(object.location[i]) > 0.0001 for i in range(3)):
            property_name = object.e2_custom_props.object_kind if property_name is None else property_name
            row = layout.row()
            row.label(text="!!! WARNING: The " + property_name + " is moved from the origin !!!")

    def warn_object_rotated(self, object, layout, property_name=None):
        if any(abs(object.rotation_euler[i]) > 0.0001 for i in range(3)):
            property_name = object.e2_custom_props.object_kind if property_name is None else property_name
            row = layout.row()
            row.label(text="!!! WARNING: The " + property_name + " is moved from the origin !!!")

    def warn_object_is_scaled(self, object, layout, property_name=None):
        if any(abs(object.scale[i] - 1.0) > 0.0001 for i in range(3)):
            property_name = object.e2_custom_props.object_kind if property_name is None else property_name
            row = layout.row()
            row.label(text="!!! WARNING: The folder is scaled !!!")

    def warn_has_children(self, object, layout):
        if len(object.children) > 0:
            row = layout.row()
            row.label(text="!!! WARNING: The " + object.e2_custom_props.object_kind + " has a child object !!!")

    def warn_no_frame_found(self, folder, layout):
        has_frame = False
        while folder != None:
            if folder.e2_custom_props.folder_defines_frame is True:
                has_frame = True
                break
            folder = folder.parent
        if has_frame is False:
            row = layout.row()
            row.label(text="!!! WARNING: Neither this nor any parent folder defines a frame !!!")

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
                row.label(text="!!! WARNING: Some parent folder already defines a rigid body !!!")
                break
            folder = folder.parent
            
    def warn_incomatible_collsion_class_with_rigid_body_moveable_flag(self, collider, layout):
        object = collider
        while object != None:
            if object.e2_custom_props.folder_defines_rigid_body is True:
                collider.e2_custom_props.collider_collision_class
                print_warn = False
                if object.e2_custom_props.rigid_body_is_moveable:
                    if collider.e2_custom_props.collider_collision_class == "STATIC_OBJECT":
                        print_warn = True
                elif collider.e2_custom_props.collider_collision_class in [
                        "COMMON_MOVEABLE_OBJECT", "HEAVY_MOVEABLE_OBJECT", "AGENT_MOTION_OBJECT"
                        ]:
                    print_warn = True
                if print_warn is True:
                    row = layout.row()
                    row.label(text="!!! WARNING: Incompatible collision class with rigid body's moveable flag !!!")
                break
            object = object.parent
            
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

    def warn_request_info_wrong_event_type(self, object_props, kind, layout):
        for i, info in enumerate(object_props.request_info_items):
            if kind == "TIMER" and info.event != "TIME_OUT":
                row = layout.row()
                row.label(text="!!! WARNING: In info #" + str(i) + ": Wrong event type for the timer !!!")
            elif kind == "SENSOR" and info.event == "TIME_OUT":
                row = layout.row()
                row.label(text="!!! WARNING: In info #" + str(i) + ": Wrong event type for the sensor !!!")

    def warn_object_is_not_folder(self, object_name, layout, property_name):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not a folder !!!")

    def warn_object_is_not_folder_with_frame(self, object_name, layout, property_name, allow_root):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "FOLDER" or object_props.folder_defines_frame is False:
            if allow_root is False or object_name != "E2_ROOT":
                row = layout.row()
                row.label(text="!!! WARNING: " + property_name + " is not a folder with a frame !!!")

    def warn_object_is_not_folder_with_timer(self, object_name, layout, property_name):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "FOLDER" or object_props.folder_defines_timer is False:
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not a folder with a timer !!!")

    def warn_object_is_not_collider(self, object_name, layout, property_name):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "COLLIDER":
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not a collider !!!")

    def warn_object_is_not_collider_with_sensor(self, object_name, layout, property_name):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "COLLIDER" or object_props.collider_defines_sensor is False:
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not a collider with a sensor !!!")

    def warn_folder_does_not_have_rigid_body(self, object_name, layout, property_name):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "FOLDER" or object_props.folder_defines_rigid_body is False:
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not a folder with a rigid_body !!!")

    def warn_not_under_rigid_body(self, folder, layout, property_name):
        while folder != None:
            if folder.e2_custom_props.folder_defines_rigid_body is True:
                return
            folder = folder.parent
        row = layout.row()
        row.label(text="!!! WARNING: " + property_name + " is not under a rigid body !!!")
    
    def warn_not_valid_import_dir(self, path, layout, property_name):
        if not os.path.isdir(path) or not os.path.isfile(os.path.join(path, "hierarchy.json")):
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not valid scene directory !!!")

    def warn_not_valid_triangle_mesh_dir(self, path, layout, property_name):
        if not os.path.isdir(path) or any(not os.path.isfile(os.path.join(path, x + ".txt")) for x in ["vertices", "indices"]):
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not valid triangle mesh directory !!!")

    def warn_is_root_folder(self, name, layout, property_name):
        if is_root_folder(get_scene_object_of_name(name)):
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is the E2_ROOT folder !!!")

    def warn_object_name_is_reserved(self, name, layout):
        reserved_names = ["FOLDER", "FRAME", "RIGID_BODY", "TIMER"]
        if name in reserved_names:
            row = layout.row()
            row.label(text="!!! WARNING: The object's name '" + name + "' is reserved !!!")
        elif name.startswith("SENSOR."):
            row = layout.row()
            row.label(text="!!! WARNING: The prefix 'SENSOR.' in object's name is reserved !!!")

    def warn_object_name_contains_reserved_character(self, name, layout):
        if '/' in name or "\\" in name:
            row = layout.row()
            row.label(text="!!! WARNING: The object's name contains '/' or '\\' !!!")


######################################################
# SCENE IMPORT AND EXPORT
######################################################


class E2SceneProps(bpy.types.PropertyGroup):

    #====================================================
    # Export
    
    export_dir: bpy.props.StringProperty(
            name="Export dir",
            description="A directory under which the scene will be exported",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
            )
    export_make_paths_relative_to_data_root: bpy.props.BoolProperty(
            name="Make exported paths relative to the data root?",
            description="Whether to make exported paths relative to the data root or not",
            default=True
            )
    export_data_root_dir: bpy.props.StringProperty(
            name="Data root dir",
            description="The root directory of all data and scenes of the E2 simulator",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
            )

    #====================================================
    # Import
    
    import_dir: bpy.props.StringProperty(
            name="Import dir",
            description="A directory from which a scene will be imported",
            default=".",
            maxlen=1000,
            subtype='DIR_PATH'
            )
    import_under_folder: bpy.props.EnumProperty(
            name="Import uder",
            description="A folder in the current scene under which to import the scene",
            items=list_of_name_of_scene_objects
            )


class E2SceneExportOperator(bpy.types.Operator):
    """ E2 scene exporter: Exports the scene under the export dir """
    bl_idname = "scene.e2_export"
    bl_label = "Export scene"
    bl_options = {'REGISTER', 'UNDO'}
    
    def execute(self, context):
        scene_props = context.scene.e2_custom_props
        try:
            print("E2 Scene exporter: Exporting scene using to dir: " + scene_props.export_dir)
            self.export_scene(
                context.scene.objects,
                str(scene_props.export_dir),
                scene_props.export_data_root_dir if scene_props.export_make_paths_relative_to_data_root is True else None
                )
            print("E2 Scene exporter: Finished successfully.")
        except Exception as e:
            print("E2 Scene exporter: Export has FAILED. Details:\n" + str(e))
        return{'FINISHED'}

    def export_scene(self, objects, output_dir, data_root_dir):
        if not os.path.isdir(output_dir):
            raise Exception("The export directory '" + output_dir + "' does not exist.")
        root_folders = []
        if "E2_ROOT" in objects and objects["E2_ROOT"].e2_custom_props.object_kind == "FOLDER":
            for object in objects:
                if object.parent is objects["E2_ROOT"] and object.e2_custom_props.object_kind == "FOLDER":
                    root_folders.append(object)
        result = { "folders":{}, "imports": {} }
        for folder in root_folders:
            if len(folder.e2_custom_props.folder_imported_from_dir) > 0:
                result["imports"][folder.name] = normalise_disk_path(folder.e2_custom_props.folder_imported_from_dir,
                                                                     data_root_dir)
            else:
                result["folders"][folder.name] = self.export_folder(folder, data_root_dir)
        result = self.clean_result(result)    
        #print(json.dumps(result, indent=4, sort_keys=True))
        with open(os.path.join(output_dir, "hierarchy.json"), "w") as f:
            json.dump(result, f, indent=4, sort_keys=True)

    def export_folder(self, folder, data_root_dir):
        result = { "content":{}, "folders":{}, "imports": {} }
        if folder.e2_custom_props.folder_defines_frame is True:
            result["content"]["FRAME"] = self.export_frame(folder)
        if folder.e2_custom_props.folder_defines_rigid_body is True:
            result["content"]["RIGID_BODY"] = self.export_rigid_body(folder)
        for child in folder.children:
            child_props = child.e2_custom_props
            if child_props.object_kind == "FOLDER":
                if len(child_props.folder_imported_from_dir) > 0:
                    result["imports"][child.name] = normalise_disk_path(child.e2_custom_props.folder_imported_from_dir,
                                                                        data_root_dir)
                else:
                    result["folders"][child.name] = self.export_folder(child, data_root_dir)
            elif child_props.object_kind == "BATCH":
                result["content"][child.name] = self.export_batch(child, data_root_dir)
            elif child_props.object_kind == "COLLIDER":
                result["content"][child.name] = self.export_collider(child, data_root_dir)
                if child_props.collider_defines_sensor is True:
                    result["content"]["SENSOR." + child.name] = self.export_sensor(child, data_root_dir)
        if folder.e2_custom_props.folder_defines_timer is True:
            result["content"]["TIMER"] = self.export_timer(folder, data_root_dir)
        return self.clean_result(result)

    def export_vector(self, v):
        return { "x": num2str(v.x), "y": num2str(v.y), "z": num2str(v.z) }

    def export_quaternion(self, q):
        return { "x": num2str(q.x), "y": num2str(q.y), "z": num2str(q.z), "w": num2str(q.w) }
    
    def export_colour(self, c):
        return { "r": num2str(c[0]), "g": num2str(c[1]), "b": num2str(c[2]), "a": num2str(c[3]) }

    def export_frame(self, object):
        old_rotation_mode = object.rotation_mode
        object.rotation_mode = "QUATERNION"
        result = {
            "object_kind": "FRAME",
            "origin": self.export_vector(object.location),
            "orientation": self.export_quaternion(object.rotation_quaternion)
        }
        object.rotation_mode = old_rotation_mode
        return result

    def export_batch(self, object, data_root_dir):
        result = {
            "object_kind": object.e2_custom_props.object_kind,
            "batch_kind": object.e2_custom_props.batch_kind
        }
        if object.e2_custom_props.batch_kind == "REGULAR_GFX_BATCH":
            result["path"] = normalise_disk_path(object.e2_custom_props.batch_reguar_disk_path, data_root_dir)
        else:
            sizes = object.scale if object.type == 'EMPTY' else 0.5 * object.dimensions
            if object.e2_custom_props.batch_kind == "GENERIC_BOX":
                result["half_sizes_along_axes"] = self.export_vector(sizes)
            elif object.e2_custom_props.batch_kind == "GENERIC_CAPSULE":
                thickness = max(0.001, min(sizes.x, sizes.y))
                result["thickness_from_central_line"] = num2str(thickness)
                distance = max(0.002, sizes.z - thickness)
                result["half_distance_between_end_points"] = num2str(distance)
                result["num_lines_per_quarter_of_circle"] = num2str(
                    max(1, object.e2_custom_props.batch_generic_num_lines_per_quarter_of_circle)
                    )
            elif object.e2_custom_props.batch_kind == "GENERIC_SPHERE":
                result["radius"] = num2str(max(0.001, min(sizes)))
                result["num_lines_per_quarter_of_circle"] = num2str(
                    max(1, object.e2_custom_props.batch_generic_num_lines_per_quarter_of_circle)
                    )
            try:
                material = object.data.materials[0].diffuse_color
                result["colour"] = self.export_colour(material)
            except Exception as e:
                result["colour"] = { "r": "0.75", "g": "0.75", "b": "1.0", "a": "1.0" }
        return result

    def export_collider(self, object, data_root_dir):
        result = {
            "object_kind": object.e2_custom_props.object_kind,
            "collider_kind": object.e2_custom_props.collider_kind
        }
        if object.e2_custom_props.collider_kind == "TRIANGLE_MESH":
            result["path"] = normalise_disk_path(object.e2_custom_props.collider_triangle_mesh_dir, data_root_dir)
        else:
            sizes = object.scale if object.type == 'EMPTY' else 0.5 * object.dimensions
            if object.e2_custom_props.collider_kind == "BOX":
                result["half_sizes_along_axes"] = self.export_vector(sizes)
            elif object.e2_custom_props.collider_kind == "CAPSULE":
                thickness = max(0.001, min(sizes.x, sizes.y))
                result["thickness_from_central_line"] = num2str(thickness)
                distance = max(0.002, sizes.z - thickness)
                result["half_distance_between_end_points"] = num2str(distance)
            elif object.e2_custom_props.collider_kind == "SPHERE":
                result["radius"] = num2str(max(0.001, min(sizes)))
            result["collision_material"] = str(object.e2_custom_props.collider_material_type)
            result["collision_class"] = str(object.e2_custom_props.collider_collision_class)
        return result

    def export_rigid_body(self, object):
        object_props = object.e2_custom_props
        result = {
            "object_kind": "RIGID_BODY",
            "is_moveable": bool2str(object_props.rigid_body_is_moveable),
        }
        if object_props.rigid_body_is_moveable is True:
            result["linear_velocity"] = self.export_vector(object_props.rigid_body_linear_velocity)
            result["angular_velocity"] = self.export_vector(object_props.rigid_body_angular_velocity)
            result["external_linear_acceleration"] = self.export_vector(object_props.rigid_body_external_linear_acceleration)
            result["external_angular_acceleration"] = self.export_vector(object_props.rigid_body_external_angular_acceleration)
        return result
    
    def export_timer(self, object, data_root_dir):
        object_props = object.e2_custom_props
        result = {
            "object_kind": "TIMER",
            "period_in_seconds": num2str(object_props.timer_period_in_seconds),
            "target_enable_level": num2str(object_props.timer_target_enable_level),
            "current_enable_level": num2str(object_props.timer_current_enable_level),
            "request_infos": self.export_request_infos(object_props.request_info_items, object.name, data_root_dir)
        }
        return result
    
    def export_sensor(self, object, data_root_dir):
        object_props = object.e2_custom_props
        result = {
            "object_kind": "SENSOR",
            "collider": object.name,
            "target_enable_level": num2str(object_props.sensor_target_enable_level),
            "current_enable_level": num2str(object_props.sensor_current_enable_level),
            "request_infos": self.export_request_infos(object_props.request_info_items, object.name, data_root_dir)
        }
        if object_props.sensor_use_exclusive_trigger_collider is True:
            result["trigger_collider"] = relative_scene_path_to_content(object_props.sensor_trigger_collider, object.name)
        return result

    def export_request_infos(self, infos, name, data_root_dir):
        result = []
        for info in infos:
            if info.kind == "INCREMENT_ENABLE_LEVEL_OF_TIMER":
                record = self.export_request_info_increment_enable_level_of_timer(info, name)
            elif info.kind == "DECREMENT_ENABLE_LEVEL_OF_TIMER":
                record = self.export_request_info_decrement_enable_level_of_timer(info, name)
            elif info.kind == "RESET_TIMER":
                record = self.export_request_info_reset_timer(info, name)
            elif info.kind == "INCREMENT_ENABLE_LEVEL_OF_SENSOR":
                record = self.export_request_info_increment_enable_level_of_sensor(info, name)
            elif info.kind == "DECREMENT_ENABLE_LEVEL_OF_SENSOR":
                record = self.export_request_info_decrement_enable_level_of_sensor(info, name)
            elif info.kind == "IMPORT_SCENE":
                record = self.export_request_info_import_scene(info, name, data_root_dir)
            elif info.kind == "ERASE_FOLDER":
                record = self.export_request_info_erase_folder(info, name)
            elif info.kind == "SET_LINEAR_VELOCITY":
                record = self.export_request_info_set_linear_velocity(info, name)
            elif info.kind == "SET_ANGULAR_VELOCITY":
                record = self.export_request_info_set_angular_velocity(info, name)
            elif info.kind == "UPDATE_RADIAL_FORCE_FIELD":
                record = self.export_request_info_update_radial_force_field(info)
            elif info.kind == "UPDATE_LINEAR_FORCE_FIELD":
                record = self.export_request_info_update_linear_force_field(info)
            elif info.kind == "LEAVE_FORCE_FIELD":
                record = {}
            else:
                raise Exception("ERROR: Unknown request info kind.")            
            record["kind"] = info.kind
            record["event"] = info.event
            result.append(record)
        return result

    def export_request_info_increment_enable_level_of_timer(self, info, name):
        result = { "timer": relative_scene_path_to_content_from_embedded(info.folder_of_timer, name, "TIMER") }
        return result

    def export_request_info_decrement_enable_level_of_timer(self, info, name):
        result = { "timer": relative_scene_path_to_content_from_embedded(info.folder_of_timer, name, "TIMER") }
        return result

    def export_request_info_reset_timer(self, info, name):
        result = { "timer": relative_scene_path_to_content_from_embedded(info.folder_of_timer, name, "TIMER") }
        return result

    def export_request_info_increment_enable_level_of_sensor(self, info, name):
        result = { "sensor": relative_scene_path_to_embedded_content(info.collider_of_sensor, name, "SENSOR." + info.collider_of_sensor) }
        return result

    def export_request_info_decrement_enable_level_of_sensor(self, info, name):
        result = { "sensor": relative_scene_path_to_embedded_content(info.collider_of_sensor, name, "SENSOR." + info.collider_of_sensor) }
        return result

    def export_request_info_import_scene(self, info, name, data_root_dir):
        result = {
            "import_dir": normalise_disk_path(info.import_dir, data_root_dir + "/import"),
            "cache_imported_scene": bool2str(info.cache_imported_scene)
        }
        if not is_root_folder(get_scene_object_of_name(info.import_under_folder)):
            result["under_folder"] = relative_scene_path_to_folder(info.import_under_folder, name)
        if not is_root_folder(get_scene_object_of_name(info.import_relocation_frame_folder)):
            result["relocation_frame"] = relative_scene_path_to_content_from_embedded(
                                                info.import_relocation_frame_folder, name, "FRAME")
        if info.apply_linear_velocity is True:
            result["linear_velocity"] = self.export_vector(info.linear_velocity)
        if info.apply_angular_velocity is True:
            result["angular_velocity"] = self.export_vector(info.angular_velocity)
        if info.apply_linear_velocity is True or info.apply_angular_velocity is True:
            if not is_root_folder(get_scene_object_of_name(info.import_motion_frame)):
                result["motion_frame"] = relative_scene_path_to_content_from_embedded(info.import_motion_frame, name, "FRAME")
                result["add_motion_frame_velocity"] = bool2str(info.import_add_motion_frame_velocity)
        return result

    def export_request_info_erase_folder(self, info, name):
        result = { "erase_folder": relative_scene_path_to_folder_from_embedded(info.erase_folder, name) }
        return result

    def export_request_info_set_linear_velocity(self, info, name):
        result = {
            "rigid_body": relative_scene_path_to_content(info.folder_of_rigid_body, name, "RIGID_BODY"),
            "linear_velocity": self.export_vector(info.linear_velocity)
        }
        return result

    def export_request_info_set_angular_velocity(self, info, name):
        result = {
            "rigid_body": relative_scene_path_to_content(info.folder_of_rigid_body, name, "RIGID_BODY"),
            "angular_velocity": self.export_vector(info.angular_velocity)
        }
        return result

    def export_request_info_update_radial_force_field(self, info):
        result = {
            "multiplier": num2str(info.radial_force_field_multiplier),
            "exponent": num2str(info.radial_force_field_exponent),
            "min_radius": num2str(info.radial_force_field_min_radius),
            "use_mass": bool2str(info.use_mass)
        }
        return result

    def export_request_info_update_linear_force_field(self, info):
        result = {
            "acceleration": self.export_vector(info.linear_force_field_acceleration),
            "use_mass": bool2str(info.use_mass)
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

    def import_scene(self, objects, import_dir, under_folder):
        pass

    def execute(self, context):
        scene_props = context.scene.e2_custom_props
        try:
            print("E2 Scene importer: Importing scene under '" + scene_props.import_under_folder +
                  "' from dir: " + scene_props.import_dir)
            self.import_scene(context.scene.objects, str(scene_props.import_dir), str(scene_props.import_under_folder))
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
        scene_props = scene.e2_custom_props
        self.draw_export(context, scene_props, layout.box())
        self.draw_import(context, scene_props, layout.box())
        
    def draw_export(self, context, scene_props, layout):
        self.warn_not_valid_dir(scene_props.export_dir, layout, "Export dir")
        if scene_props.export_make_paths_relative_to_data_root is True:
            self.warn_not_valid_dir(scene_props.export_data_root_dir, layout, "Data root dir")

        row = layout.row()
        row.prop(scene_props, "export_dir")

        box = layout.box()
        row = box.row()
        row.prop(scene_props, "export_make_paths_relative_to_data_root")
        row = box.row()
        row.prop(scene_props, "export_data_root_dir")

        row = layout.row()
        row.operator("scene.e2_export")

    def draw_import(self, context, scene_props, layout):
        self.warn_object_is_not_folder(scene_props.import_under_folder, layout, "Import under")
        self.warn_not_valid_import_dir(scene_props.import_dir, layout, "Import dir")

        row = layout.row()
        row.prop(scene_props, "import_dir")

        row = layout.row()
        row.prop(scene_props, "import_under_folder")

        row = layout.row()
        row.operator("scene.e2_import")

    # == warnings ======================================================================

    def warn_object_is_not_folder(self, object_name, layout, property_name):
        object_props = e2_custom_props_of(object_name)
        if object_props.object_kind != "FOLDER":
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not a folder !!!")

    def warn_not_valid_dir(self, path, layout, property_name):
        if not os.path.isdir(path):
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not valid directory !!!")
            
    def warn_not_valid_import_dir(self, path, layout, property_name):
        if not os.path.isdir(path) or not os.path.isfile(os.path.join(path, "hierarchy.json")):
            row = layout.row()
            row.label(text="!!! WARNING: " + property_name + " is not valid scene directory !!!")
            
            
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
    bpy.utils.register_class(E2SceneProps)
    bpy.utils.register_class(E2SceneExportOperator)
    bpy.utils.register_class(E2SceneImportOperator)
    bpy.utils.register_class(E2SceneIOPanel)
    bpy.types.Object.e2_custom_props = bpy.props.PointerProperty(type=E2ObjectProps)
    bpy.types.Scene.e2_custom_props =  bpy.props.PointerProperty(type=E2SceneProps)


def unregister():
    del bpy.types.Scene.e2_custom_props
    del bpy.types.Object.e2_custom_props
    bpy.utils.unregister_class(E2SceneIOPanel)
    bpy.utils.unregister_class(E2SceneImportOperator)
    bpy.utils.unregister_class(E2SceneExportOperator)
    bpy.utils.unregister_class(E2SceneProps)
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
