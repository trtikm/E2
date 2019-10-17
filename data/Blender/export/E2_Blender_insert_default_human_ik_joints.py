import bpy


armature_name = "agent/TODO"

armature = bpy.data.objects[armature_name]
bones = armature.pose.bones

bones["eye.L"]["e2_joint_axis"] = [2, 0]
bones["eye.L"]["e2_joint_axis_in_parent_space"] = [1, 0]
bones["eye.L"]["e2_joint_max_angle"] = [0.8726646, 0.5235988]
bones["eye.L"]["e2_joint_stiffness_with_parent_bone"] = [1.0, 1.0]
bones["eye.L"]["e2_joint_max_angular_speed"] = [12.56637, 12.56637]

bones["eye.R"]["e2_joint_axis"] = [2, 0]
bones["eye.R"]["e2_joint_axis_in_parent_space"] = [1, 0]
bones["eye.R"]["e2_joint_max_angle"] = [0.8726646, 0.5235988]
bones["eye.R"]["e2_joint_stiffness_with_parent_bone"] = [1.0, 1.0]
bones["eye.R"]["e2_joint_max_angular_speed"] = [12.56637, 12.56637]

bones["head"]["e2_joint_axis"] = [1, 2]
bones["head"]["e2_joint_axis_in_parent_space"] = [1, 0]
bones["head"]["e2_joint_max_angle"] = [2.0943951, 1.3962634]
bones["head"]["e2_joint_stiffness_with_parent_bone"] = [1.0, 1.0]
bones["head"]["e2_joint_max_angular_speed"] = [6.2831853, 6.2831853]

bones["neck"]["e2_joint_axis"] = [1, 2]
bones["neck"]["e2_joint_axis_in_parent_space"] = [1, 0]
bones["neck"]["e2_joint_max_angle"] = [0.5235988, 0.5235988]
bones["neck"]["e2_joint_stiffness_with_parent_bone"] = [1.0, 1.0]
bones["neck"]["e2_joint_max_angular_speed"] = [3.1415927, 3.1415927]
