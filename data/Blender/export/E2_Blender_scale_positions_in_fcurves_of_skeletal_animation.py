"""
The script scales all xyz-positions of all bones in all keyframes of an active action in the ActionEditor.
Before running the scripr do the following (these step are assumed in the code!!!):
    1. Select the source armature (and (optionally) apply the scale: Object/Apply/Scale)
    2. In the ActionEditor (of the Dope Sheet editor) select the Action to be processed
    3. Unfold one of the bones in the ActionEditor and look for order of rotation, position,
       and scale fcurves. NOTE: some fcurves may be missing (e.g. scale).
    4. Update constants in the 'config' section below according to selected data in the previous steps.
    5. Update the constant POS_SCALE_XYZ below to a desired scale to be applied to the positions.
"""

import bpy
import mathutils
import math


################################################################
# CONFIG SECTION - update the values to desired ones !!!

armature_name = "Agent"

FCURVE_INFO_SIZE_PER_BONE = 4 + 3 + 3   # rot[wxyz] + pos[xyz] + scale[xyz]
FCURVE_ROT_OFFSET = 0
FCURVE_POS_OFFSET = 4
FCURVE_SCALE_OFFSET = 7

POS_SCALE_XYZ = 0.01

debug_mode = True

################################################################


armature = bpy.data.objects[armature_name]
action = armature.animation_data.action
fcurves = action.fcurves
for bone_idx in range(len(fcurves) // FCURVE_INFO_SIZE_PER_BONE):
    for coord_idx in range(3):
        for point in fcurves[bone_idx * FCURVE_INFO_SIZE_PER_BONE + FCURVE_POS_OFFSET + coord_idx].keyframe_points:
            if debug_mode is True:
                print("[SCALING] timepoint=" + str(point.co[0]) + ", pos[" + str(coord_idx) + "]=" + str(point.co[1]))
            point.co[1] *= POS_SCALE_XYZ
