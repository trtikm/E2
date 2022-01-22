"""
SCRIPT FOR COPYING ANIMATIONS BETWEEN (INCOMPATIBLE) ARMATURES
==============================================================

The script copies all keyframes from the active animation of a source armature to the active
action of a destination armature. Use the 'ActionEditor' to activate animations to the
armatures.

The script is not fully automated. The text below describes manual steps to be performed
before running the script.


1. Preparation of the source armature
-------------------------------------

A souce armature object can have a scale. If the animation looks good though, it would
mean that its positions (in individual keyframes) are scaled too. Since we require the
destination armature does not have scale, we have to remove the scale from the source
animation before we move it to the destination armature. We do so using the script

        E2_Blender_scale_positions_in_fcurves_of_skeletal_animation.py


2. Maximise spatial alignment of source and destination armature
----------------------------------------------------------------

In order to obtain maximal quality of the copied animation from the source animation
it is essential to spatially align both armatures. A valid operation for the source
armature is uniform scaling in all axes. Do not forget though to apply the scale to
the armature before running the script (i.e. to eliminate the scale by applying it
to bones of the armature). A valid operation for the destination armature is to use
IK bones to pose the armature to the most aligned pose with the source armature.

(Optional) Once you aligned the armatures, you can save the pose of the destination
armature in a single keyframe in an auxiliary animation (use the 'ActionEditor' to
create a new animation and insert the current pose there).


3. Create empty action for the destination armature
---------------------------------------------------

Select the destination armature and in the ActionEditor (of the Dope Sheet editor) click
on 'New' button to create a new action for the destinaton armature. Optionally give it
a proper name. This action must be active for the destination armature at the time of
running this script.


4. Setup the 'config' section of this script
--------------------------------------------

In the constructor (i.e. method '__init__') of the class CopySkeletalAnimation there is
clearly marked a 'CONFIG SECTION'. Variables there should be set to values related to the
considered source and target armatures. The current values serve only as an example to see
how the values may possibly look like.

"""


import bpy
import mathutils
import math


class CopySkeletalAnimation:
    def __init__(self):
        #############################################################################################
        # CONFIG SECTION - set variables below to desired values before running the script !!!

        self.src_armature_name = "Armature"
        self.dst_armature_name = "agent/TODO"

        self.bones_map = {
            # A map from all bones of the source armature to the corresponding bones of the destination armature.
            # The map thus defines where to copy positions and rotations of what source bones.

            "TODO_Hips": "lower_body",
            "TODO_Spine": "middle_body",
            "TODO_Spine2": "upper_body",
            "TODO_Neck": "neck",
            "TODO_Head": "head",

            "TODO_LeftShoulder": "to_arm.L",
            "TODO_LeftArm": "upper_arm.L",
            "TODO_LeftForeArm": "lower_arm.L",
            "TODO_LeftHand": "arm.IK.L",

            "TODO_LeftHandPinky1": "upper_finger4.L",
            "TODO_LeftHandPinky2": "middle_finger4.L",
            "TODO_LeftHandPinky3": "lower_finger4.L",

            "TODO_LeftHandRing1": "upper_finger3.L",
            "TODO_LeftHandRing2": "middle_finger3.L",
            "TODO_LeftHandRing3": "lower_finger3.L",

            "TODO_LeftHandMiddle1": "upper_finger2.L",
            "TODO_LeftHandMiddle2": "middle_finger2.L",
            "TODO_LeftHandMiddle3": "lower_finger2.L",

            "TODO_LeftHandIndex1": "upper_finger1.L",
            "TODO_LeftHandIndex2": "middle_finger1.L",
            "TODO_LeftHandIndex3": "lower_finger1.L",

            "TODO_LeftHandThumb2": "upper_thumb.L",
            "TODO_LeftHandThumb3": "lower_thumb.L",

            "TODO_RightShoulder": "to_arm.R",
            "TODO_RightArm": "upper_arm.R",
            "TODO_RightForeArm": "lower_arm.R",
            "TODO_RightHand": "arm.IK.R",

            "TODO_RightHandPinky1": "upper_finger4.R",
            "TODO_RightHandPinky2": "middle_finger4.R",
            "TODO_RightHandPinky3": "lower_finger4.R",

            "TODO_RightHandRing1": "upper_finger3.R",
            "TODO_RightHandRing2": "middle_finger3.R",
            "TODO_RightHandRing3": "lower_finger3.R",

            "TODO_RightHandMiddle1": "upper_finger2.R",
            "TODO_RightHandMiddle2": "middle_finger2.R",
            "TODO_RightHandMiddle3": "lower_finger2.R",

            "TODO_RightHandIndex1": "upper_finger1.R",
            "TODO_RightHandIndex2": "middle_finger1.R",
            "TODO_RightHandIndex3": "lower_finger1.R",

            "TODO_RightHandThumb2": "upper_thumb.R",
            "TODO_RightHandThumb3": "lower_thumb.R",

            "TODO_LeftUpLeg": "upper_leg.L",
            "TODO_LeftLeg": "lower_leg.L",
            "TODO_LeftFoot": "leg.IK.L",
            "TODO_LeftToeBase": "lower_foot.L",

            "TODO_RightUpLeg": "upper_leg.R",
            "TODO_RightLeg": "lower_leg.R",
            "TODO_RightFoot": "leg.IK.R",
            "TODO_RightToeBase": "lower_foot.R",
        }

        self.dst_parents_override = {
            # This map allows you to change parents of bones of the destination armature. This is perhaps
            # useful only of IK bones which often do not follow the standard parent-child chain of bones.

            "arm.IK.L": "lower_arm.L",
            "arm.IK.R": "lower_arm.R",

            "leg.IK.L": "lower_leg.L",
            "leg.IK.R": "lower_leg.R",
        }

        self.pole_targets = {
            # This map is dedicated for computation of positions of pole targets of the destination armature.
            # The keys "parent" and "child" map to names of bones forming a joint the pole target is associated
            # with. The bones typically are in the parent-child relationship, but that does not have to be the case.
            # The key "distance" defines a desired distance of the pole target position from the joint. The sigh
            # of the distance defines on what side of the joint the pole target should be placed. Typically, the
            # distance is opposite of pole targets of elbow and knee, because they bend in opposite directions.

            "arm.pole.IK.L": {"parent": "upper_arm.L", "child": "lower_arm.L", "distance": 1.0},
            "arm.pole.IK.R": {"parent": "upper_arm.R", "child": "lower_arm.R", "distance": 1.0},

            "leg.pole.IK.L": {"parent": "upper_leg.L", "child": "lower_leg.L", "distance": -1.0},
            "leg.pole.IK.R": {"parent": "upper_leg.R", "child": "lower_leg.R", "distance": -1.0},
        }

        # The script automatically mutes all IK constraints of all bones of the destination armature before
        # start of copying positions and rotations of bones. Set this variable to True if you want the script
        # to restore the original mute states of the IK constraints.
        self.restore_mute_states_of_ik_constraints = False

        self.debug_mode = False     # Prints more progress messages to the console when set to True.

        #############################################################################################

    def _initialise_dependent_variables(self):
        self.src_armature = bpy.data.objects[self.src_armature_name]
        self.dst_armature = bpy.data.objects[self.dst_armature_name]

        self.src_parents_map = {bone.name: bone.parent.name for bone in self.src_armature.data.bones if bone.parent is not None}
        self.dst_parents_map = {bone.name: bone.parent.name for bone in self.dst_armature.data.bones if bone.parent is not None}
        self.dst_parents_map.update(self.dst_parents_override)

        self.src_set_of_bones_to_consider = set(self.bones_map.keys())
        self.dst_set_of_bones_to_consider = topologically_sorted_bone_names(set(self.bones_map.values()), self.dst_parents_map)

        self.src_base_pose_frames = get_coord_systems_of_all_bones_of_base_pose_of_armature(self.src_armature)
        self.dst_base_pose_frames = {bone.name: CoordSystem.make_from_matrix44(bone.matrix) for bone in self.dst_armature.pose.bones}

        self.src_base_pose_frames_in_parent_frames = {
            bone: self.src_base_pose_frames[parent].transform_to_my_frame(self.src_base_pose_frames[bone])
            for bone, parent in self.src_parents_map.items()
            }
        self.dst_base_pose_frames_in_parent_frames = {
            bone: self.dst_base_pose_frames[parent].transform_to_my_frame(self.dst_base_pose_frames[bone])
            for bone, parent in self.dst_parents_map.items()
            }

        self.time_points = {round(point.co[0]) for fcurve in self.src_armature.animation_data.action.fcurves for point in fcurve.keyframe_points}

    def run(self):
        if self.debug_mode is True:
            print("Checking for validity of script preconditions.")

        if len(bpy.context.selected_objects) != 1:
            print("ERROR: Exactly 1 object must be selected. Namely, the destination armature.")
            return
        if bpy.context.selected_objects[0].type != "ARMATURE":
            print("ERROR: The selected object is not an armature. Please, select the destination armature.")
            return
        if bpy.context.selected_objects[0].name != self.dst_armature_name:
            print("ERROR: The selected armature is not the destination armature.")
            return
        if self.src_armature_name not in bpy.data.objects:
            print("ERROR: Cannot find the source armature '" + str(self.src_armature_name) + "'.")
            return
        if self.src_armature_name not in bpy.data.objects:
            print("ERROR: Cannot find the source armature '" + str(self.src_armature_name) + "'.")
            return
        if bpy.data.objects[self.src_armature_name].type != "ARMATURE":
            print("ERROR: The source armature '" + str(self.src_armature_name) + "' is actually not an armature.")
            return
        if len(self.bones_map) == 0:
            print("Nothing to do. Check Config.bones_map.")
            return

        if self.debug_mode is True:
            print("Initialising state variables.")

        self._initialise_dependent_variables()

        if self.debug_mode is True:
            print("Building backups: current frame and mute state of IK bone of destination armature.")

        frame_current_backup = bpy.context.scene.frame_current
        bone_ik_mute_backup = {}
        for bone in self.dst_armature.pose.bones:
            if "IK" in bone.constraints:
                bone_ik_mute_backup[bone.name] = bone.constraints["IK"].mute

        if self.debug_mode is True:
            print("Switching Blender to POSE mode, selecting 'considered' bones, and disabling IK constraints of the destination armature.")

        bpy.ops.object.mode_set(mode='POSE')
        for bone in self.dst_armature.data.bones:
            bone.select = bone.name in self.dst_set_of_bones_to_consider or bone.name in self.pole_targets
        for bone_name in bone_ik_mute_backup:
            self.dst_armature.pose.bones[bone_name].constraints["IK"].mute = True

        if self.debug_mode is True:
            print("Starting copying of individual keyframes.")

        for time_point in self.time_points:

            print("Copying keyframe at time point " + str(time_point))

            if self.debug_mode is True:
                print("    Setting (all) armatures to the poses of the processed keyframe.")

            bpy.context.scene.frame_set(time_point)     # applies to all armatures

            if self.debug_mode is True:
                print("    Computing motion differences of bones from the base to the current pose of the source armature.")

            movements_in_world = {}
            src_current_pose_frames = {bone.name: CoordSystem.make_from_matrix44(bone.matrix) for bone in self.src_armature.pose.bones}
            for bone_name in self.src_set_of_bones_to_consider:
                if bone_name in self.src_parents_map:
                    parent_bone_name = self.src_parents_map[bone_name]
                    bone_frame_in_parent = src_current_pose_frames[parent_bone_name].transform_to_my_frame(src_current_pose_frames[bone_name])
                    difference_in_parent = MotionDifference.make_from_frames(self.src_base_pose_frames_in_parent_frames[bone_name], bone_frame_in_parent)
                    difference_in_world = difference_in_parent.transform_from_frame(self.src_base_pose_frames[bone_name])
                else:
                    difference_in_world = MotionDifference.make_from_frames(self.src_base_pose_frames[bone_name], src_current_pose_frames[bone_name])
                movements_in_world[self.bones_map[bone_name]] = difference_in_world

            if self.debug_mode is True:
                print("    Clearing transformation matrices of selected bones of the destination armature.")

            bpy.ops.pose.transforms_clear()     # applies only to selected bones of the dst_armature

            if self.debug_mode is True:
                print("    Applying computed motion differences to corresponding bones of the destination armature.")

            for bone_name in self.dst_set_of_bones_to_consider:
                if bone_name in self.dst_parents_map:
                    parent_bone_name = self.dst_parents_map[bone_name]
                    movement_in_parent = movements_in_world[bone_name].transform_to_frame(self.dst_base_pose_frames[parent_bone_name])
                    moved_bone_frame_in_parent = movement_in_parent.apply_to_frame(self.dst_base_pose_frames_in_parent_frames[bone_name])
                    parent_bone_frame = CoordSystem.make_from_matrix44(self.dst_armature.pose.bones[parent_bone_name].matrix)
                    frame_in_world = parent_bone_frame.transform_from_my_frame(moved_bone_frame_in_parent)
                else:
                    frame_in_world = movements_in_world[bone_name].apply_to_frame(self.dst_base_pose_frames[bone_name])
                self.dst_armature.pose.bones[bone_name].matrix = frame_in_world.get_matrix_from()

                bpy.ops.anim.keyframe_insert_menu(type='LocRotScale')

            if self.debug_mode is True:
                if len(self.pole_targets) > 0:
                    print("    Computing positions of pole targets.")

            for bone_name, info in self.pole_targets.items():
                parent_bone = self.dst_armature.pose.bones[info["parent"]]
                child_bone = self.dst_armature.pose.bones[info["child"]]
                joint_location = (child_bone.matrix @ mathutils.Vector((0.0, 0.0, 0.0, 1.0))).to_3d()
                parent_y_vector = (parent_bone.matrix.to_3x3() @ mathutils.Vector((0.0, -1.0, 0.0))).normalized()
                child_y_vector = (child_bone.matrix.to_3x3() @ mathutils.Vector((0.0, 1.0, 0.0))).normalized()
                if parent_y_vector.angle(child_y_vector, math.pi) >= math.pi - 5.0 * (math.pi / 180.0):
                    pole_vector = mathutils.Vector((0.0, info["distance"], 0.0))
                else:
                    pole_vector = info["distance"] * (parent_y_vector + child_y_vector).normalized()
                self.dst_armature.pose.bones[bone_name].matrix = mathutils.Matrix.Translation(joint_location + pole_vector)
                bpy.ops.anim.keyframe_insert_menu(type='LocRotScale')

        if self.debug_mode is True:
            print("    Restoring current frame.")
        bpy.context.scene.frame_set(frame_current_backup)
        if self.restore_mute_states_of_ik_constraints:
            if self.debug_mode is True:
                print("    Restoring mute states of IK bones.")
            for bone_name, state in bone_ik_mute_backup.items():
                self.dst_armature.pose.bones[bone_name].constraints["IK"].mute = state


########################################################################################################################
# UTILITIES
########################################################################################################################


class CoordSystem:
    """Represents a FRAME of reference in 3d space."""

    def __init__(self,
                 position,       # A 3d 'mathutils.Vector'; Represents frame's origin.
                 orientation     # A unit (normalised) 'mathutils.Quaternion'; Represents frame's rotation.
                 ):
        assert isinstance(position, mathutils.Vector)
        assert isinstance(orientation, mathutils.Quaternion)
        self.position = position
        self.orientation = orientation
        self._matrix_from = None
        self._matrix_to = None

    def get_matrix_from(self):
        if self._matrix_from is None:
            self._matrix_from = mathutils.Matrix.Translation(self.position) @ self.orientation.to_matrix().to_4x4()
        return self._matrix_from

    def get_matrix_to(self):
        if self._matrix_to is None:
            self._matrix_to = self.orientation.to_matrix().transposed().to_4x4() @ mathutils.Matrix.Translation(-self.position)
        return self._matrix_to

    def transform_to_my_frame(self, coord_system):
        assert isinstance(coord_system, CoordSystem)
        return CoordSystem.make_from_matrix44(self.get_matrix_to() @ coord_system.get_matrix_from())

    def transform_from_my_frame(self, coord_system):
        assert isinstance(coord_system, CoordSystem)
        return CoordSystem.make_from_matrix44(self.get_matrix_from() @ coord_system.get_matrix_from())

    @staticmethod
    def make_from_matrix44(matrix_4x4):
        assert isinstance(matrix_4x4, mathutils.Matrix)
        return CoordSystem(
            (matrix_4x4 @ mathutils.Vector((0.0, 0.0, 0.0, 1.0))).to_3d(),
            matrix_4x4.to_3x3().to_quaternion().normalized()
            )

    def __str__(self):
        return "CoordSystem{pos=" + str(self.position) + ", rot=" + str(self.orientation) + "}"


class MotionDifference:

    def __init__(self,
                 translation,   # A 3d vector
                 axis,          # A non-zero 3d vector
                 angle          # In radians
                 ):
        assert isinstance(translation, mathutils.Vector)
        assert isinstance(axis, mathutils.Vector)
        self.translation = translation
        self.axis = axis
        self.angle = angle

    def transform_from_frame(self, coord_system):
        assert isinstance(coord_system, CoordSystem)
        rotation = coord_system.orientation.to_matrix()
        return MotionDifference(rotation @ self.translation, rotation @ self.axis, self.angle)

    def transform_to_frame(self, coord_system):
        assert isinstance(coord_system, CoordSystem)
        rotation = coord_system.orientation.to_matrix().transposed()
        return MotionDifference(rotation @ self.translation, rotation @ self.axis, self.angle)

    def apply_to_frame(self, coord_system):
        assert isinstance(coord_system, CoordSystem)
        rotation = mathutils.Quaternion(self.axis, self.angle).to_matrix() @ coord_system.orientation.to_matrix()
        return CoordSystem(coord_system.position + self.translation, rotation.to_quaternion().normalized())

    @staticmethod
    def make_from_frames(start_coord_system, end_coord_system):
        assert isinstance(start_coord_system, CoordSystem)
        assert isinstance(end_coord_system, CoordSystem)
        rotation = end_coord_system.orientation.to_matrix() @ start_coord_system.orientation.to_matrix().transposed()
        axis, angle = rotation.to_quaternion().to_axis_angle()
        return MotionDifference(end_coord_system.position - start_coord_system.position, axis, angle)

    def __str__(self):
        return "MotionDifference{trans=" + str(self.translation) + ", axis=" + str(self.axis) + ", angle=" + str(self.angle) + "}"


def topologically_sorted_bone_names(bones_set, parents):
    """
    :param bones_set: A set of bone names to be considered in sorting
    :param parents: A map from bone names to names of their parent bones (for those who actually have a parent)
    :return: A list of bones in the bones_set sorted topologically from patents to children, i.e. for each bone
             all its parents (direct and indirect) precede the bone in the list.
    """
    work_set = set()
    for bone_name in bones_set:
        work_set.add(bone_name)
        while bone_name in parents:
            bone_name = parents[bone_name]
            work_set.add(bone_name)
    result = []
    processed = set()
    while len(work_set) > 0:
        to_remove = set()
        for bone_name in work_set:
            if bone_name not in parents or parents[bone_name] in processed:
                if bone_name in bones_set:
                    result.append(bone_name)
                processed.add(bone_name)
                to_remove.add(bone_name)
        work_set = work_set - to_remove
    return result


def get_coord_systems_of_all_bones_of_base_pose_of_armature(armature):
    """
    For each bone of the base pose of the armature computes a corresponding coord system in the "world space",
    i.e. the coord system no longer depend on the parent-child hierarchy of bones.
    :return: A dictionary from name of bones of the base pose corresponding coord systems.
    """
    result = {}
    for bone in armature.data.bones:
        bones_list = []
        xbone = bone
        while xbone:
            bones_list.append(xbone)
            xbone = xbone.parent
        bones_list.reverse()

        transform_matrix = mathutils.Matrix()
        transform_matrix.identity()
        parent_tail = mathutils.Vector((0.0, 0.0, 0.0))
        for xbone in bones_list:
            bone_to_base_matrix = CoordSystem(parent_tail + xbone.head, xbone.matrix.to_quaternion()).get_matrix_to()
            transform_matrix = bone_to_base_matrix @ transform_matrix
            parent_tail = bone_to_base_matrix @ (parent_tail + xbone.tail).to_4d()
            parent_tail.resize_3d()

        result[bone.name] = CoordSystem.make_from_matrix44(transform_matrix.inverted())
    return result


########################################################################################################################
# RUNNING THE COPY
########################################################################################################################


print("=== Running CopySkeletalAnimation =========================================")
CopySkeletalAnimation().run()
print("=== Done CopySkeletalAnimation ============================================")
