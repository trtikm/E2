"""
The script first computes for each bone a rotation to be applied to rotation of the corresponding
bone in each keyframe in the animation, and then it applies the rotations to the bones in all
keyframes. A rotation to be applied to a bone is computed as a quaternion difference between
bone's quaternion in the first keyframe and the last keyframe in the animation. So, it implies
that you are supposed to create an auxiliary last keyframe, as the copy of the first one, where
you fix rotations of bones.
Before you run the script edit the 'CONFIG SECTION' below.
After running the script delete the auxiliary last keyframe from the animation.
"""

import bpy
import mathutils
import math


################################################################
# CONFIG SECTION - update the values to desired ones !!!

armature_name = "<your-armature-name>"

FCURVE_INFO_SIZE_PER_BONE = 3 + 4 + 3   # pos[xyz] + rot[wxyz] + scale[xyz]
FCURVE_POS_OFFSET = 0
FCURVE_ROT_OFFSET = 3
FCURVE_SCALE_OFFSET = 7

################################################################

print("===============================================================================")

armature = bpy.data.objects[armature_name]
action = armature.animation_data.action
fcurves = action.fcurves

rot_diffs = {}
for k in range(len(fcurves) // FCURVE_INFO_SIZE_PER_BONE):
    rot_idx = k * FCURVE_INFO_SIZE_PER_BONE + FCURVE_ROT_OFFSET
    src_ori = mathutils.Quaternion((
            fcurves[rot_idx + 0].keyframe_points[0].co[1],
            fcurves[rot_idx + 1].keyframe_points[0].co[1],
            fcurves[rot_idx + 2].keyframe_points[0].co[1],
            fcurves[rot_idx + 3].keyframe_points[0].co[1]
            ))
    dst_ori = mathutils.Quaternion((
            fcurves[rot_idx + 0].keyframe_points[-1].co[1],
            fcurves[rot_idx + 1].keyframe_points[-1].co[1],
            fcurves[rot_idx + 2].keyframe_points[-1].co[1],
            fcurves[rot_idx + 3].keyframe_points[-1].co[1]
            ))                
    rot_diffs[rot_idx] = src_ori.rotation_difference(dst_ori)

for k in range(len(fcurves) // FCURVE_INFO_SIZE_PER_BONE):
    rot_idx = k * FCURVE_INFO_SIZE_PER_BONE + FCURVE_ROT_OFFSET
    num_frames = len(fcurves[rot_idx].keyframe_points)
    for i in range(num_frames - 1):
        ori = mathutils.Quaternion((
                fcurves[rot_idx + 0].keyframe_points[i].co[1],
                fcurves[rot_idx + 1].keyframe_points[i].co[1],
                fcurves[rot_idx + 2].keyframe_points[i].co[1],
                fcurves[rot_idx + 3].keyframe_points[i].co[1]
                ))
        ori_rotated = (ori * rot_diffs[rot_idx]).normalized()
        fcurves[rot_idx + 0].keyframe_points[i].co[1] = ori_rotated.w
        fcurves[rot_idx + 1].keyframe_points[i].co[1] = ori_rotated.x
        fcurves[rot_idx + 2].keyframe_points[i].co[1] = ori_rotated.y
        fcurves[rot_idx + 3].keyframe_points[i].co[1] = ori_rotated.z
