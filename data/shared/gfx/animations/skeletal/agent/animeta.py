from gettext import translation
import os
import sys
import numpy
import math
import argparse
import json
import traceback
import subprocess


def _get_script_description():
    return """
The script simplifies construction of meta data for skeletal animations.
"""


def _parse_command_line_options():
    parser = argparse.ArgumentParser(
        description=_get_script_description(),
        )
    parser.add_argument(
        "command", type=str,
        help="A name of a command to be executed. Use the command 'help' to see a list of all available commands "
             "together with their descriptions."
        )
    parser.add_argument(
        "arguments", type=str, nargs=argparse.ZERO_OR_MORE,
        help="Arguments for the command."
        )
    cmdline = parser.parse_args()
    return cmdline


class Config:

    instance = None

    class State:
        def __init__(self, pathname):
            self.work_dir = pathname
            self.anim_dir = None
            self.anim_dir2 = None
            self.bone_idx = 0
            self.pivot = [0.0, 0.0, 0.83]
            self.shape = "capsule"
            self.length = 0.615
            self.radius = 0.2
            self.weight = 1.0
            self.mass_inverted = 1.0 / 60.0
            self.inertia_tensor_inverted = [
                0.0, 0.0, 0.0,        # Row 0
                0.0, 0.0, 0.0,        # Row 1
                0.0, 0.0, 1.0         # Row 2
            ]
            self.angle = math.pi / 4.0
            self.vec_down = [0.0, 0.0, -1.0]
            self.vec_fwd = [0.0, -1.0, 0.0]
            self.vec_fwd_end = [0.0, -1.0, 0.0]
            self.vec_down_mult = -1.0
            self.vec_fwd_mult = -1.0
            self.min_linear_speed = 0.5
            self.max_linear_speed = 1.5
            self.max_linear_accel = 20.0
            self.min_angular_speed = 0.0
            self.max_angular_speed = 2.0 * math.pi / 3.0
            self.max_angular_accel = 20.0
            self.bones_filter = [
                "lower_body",
                "middle_body",
                "upper_body",
                "neck",
                "head",
                "to_arm.L",
                "upper_arm.L",
                "lower_arm.L",
                "to_arm.R",
                "upper_arm.R",
                "lower_arm.R",
                "upper_leg.L",
                "lower_leg.L",
                "upper_leg.R",
                "lower_leg.R",
                "hand.L",
                "upper_foot.L",
                "lower_foot.L",
                "hand.R",
                "upper_foot.R",
                "lower_foot.R",
            ]
            self.bones = {
                # "neck": False,
                # "head": False,
                "eye.L": True,
                # "eye.R": True
            }
            self.distance_threshold = 1.0
            self.delta_time_in_seconds = 0.25
            self.motion_error_multiplier = 1.0
            self.gravity_accel = -9.81
            self.vec_transition = [0.0, 0.0, 0.0]
            self.vec_velocity = [0.0, 0.0, 0.0]
            self.debug_mode = True

        def typeof(self, var_name):
            default_value = Config.State("").get(var_name)
            if default_value is None:
                default_value = ""
            return type(default_value)

        def elemtypeof(self, var_name):
            default_value = Config.State("").get(var_name)
            if default_value is None or not isinstance(default_value, list):
                return None
            return type(default_value[0])

        def has(self, var_name):
            return var_name in self.__dict__

        def get(self, var_name):
            if self.has(var_name) is False:
                raise Exception("Unknown state variable '" + var_name + "'.")
            return self.__dict__[var_name]

        def set(self, var_name, value):
            if type(value) != self.typeof(var_name):
                raise Exception("Wrong value type '" + str(type(value)) + "'. Expected type is '" + str(self.typeof(var_name)) + "'.")
            self.__dict__[var_name] = value

        def value_from_list_of_strings(self, var_name, list_of_strings):
            assert type(list_of_strings) == list and len(list_of_strings) > 0
            var_type = self.typeof(var_name)
            if var_type == list:
                if type(list_of_strings) != list:
                    raise Exception("Wrong value type. Expected type a list.")
                if self.elemtypeof(var_name) is float:
                    return [float(x) for x in list_of_strings]
                else:
                    result = []
                    for x in list_of_strings:
                        if x == "@@":
                            for y in self.get(var_name):
                                result.append(y)
                        else:
                            result.append(x)
                    return result
            else:
                if len(list_of_strings) > 1:
                    raise Exception("Wrong value type. Expected a single value, but passed a list.")
                if var_type == str:
                    if var_name in ["work_dir"]:
                        return os.path.abspath(list_of_strings[0].replace("@@", self.get(var_name)))
                    return list_of_strings[0]
                if var_type == int:
                    return int(list_of_strings[0])
                if var_type == float:
                    return float(list_of_strings[0])
                raise Exception("Wrong type of the value '" + str(list_of_strings[0]) + "'. Expected type is '" + str(var_type) + "'.")

        def vars(self):
            return self.__dict__.keys()

        @staticmethod
        def load(pathname):
            state = Config.State(os.path.dirname(pathname))
            if not os.path.isfile(pathname):
                return state
            with open(pathname, "r") as f:
                state_json = json.load(f)
            for field in state.__dict__:
                if field in state_json:
                    state.__dict__[field] = state_json[field]
            return state

        def save(self, pathname):
            os.makedirs(os.path.dirname(pathname), exist_ok=True)
            with open(pathname, "w") as f:
                json.dump(self.__dict__, f, indent=4, sort_keys=True)

    @staticmethod
    def initialise(cmdline, commands, help_commands):
        Config.instance = Config(cmdline, commands, help_commands)

    def __init__(self, cmdline, commands, help_commands):
        self.cmdline = cmdline
        self.commands = commands
        self.help_commands = help_commands
        self.script_dir = os.path.abspath(os.path.dirname(__file__))
        self.state_pathname = os.path.join(self.script_dir, "animeta_state.json")
        self.state = Config.State.load(self.state_pathname)
        self.save_state = True


def _float_to_string(number, precision=6):
    assert isinstance(number, float)
    return format(number, "." + str(precision) + "f")


def _vector_length(vector):
    return math.sqrt(numpy.dot(vector, vector))


def _normalised_vector(vector):
    l = _vector_length(vector)
    if l < 0.000001:
        return [0.0, 0.0, 1.0]
    return (1.0/l) * numpy.array(vector)


def _normalised_quaternion(q):
    l = _vector_length(q)
    if l < 0.000001:
        return [1.0, 0.0, 0.0, 0.0]
    return (1.0/l) * numpy.array(q)


def _scale_vector(scalar, vector):
    return scalar * numpy.array(vector)


def _add_vectors(u, v):
    return numpy.array(u) + numpy.array(v)


def _subtract_vectors(u, v):
    return numpy.array(u) - numpy.array(v)


def _dot_product(u, v):
    return numpy.dot(u, v)


def _cross_product(u, v):
    return numpy.cross(u, v)


def _multiply_matrix_by_matrix(A, B):
    return numpy.dot(A, B)


def _multiply_matrix_by_vector(A, u):
    return numpy.dot(A, u)


def _transform_point(A, p):
    return list(_multiply_matrix_by_vector(A, list(p) + [1.0]))[:-1]


def _transform_vector(A, u):
    return list(_multiply_matrix_by_vector(A, list(u) + [0.0]))[:-1]


def _distance_between_points(p1, p2):
    return _vector_length(_subtract_vectors(p2, p1))


def _axis_angle_to_quaternion(axis_unit_vector, angle_in_radians):
    t = math.sin(0.5 * angle_in_radians)
    return [math.cos(0.5 * angle_in_radians), t * axis_unit_vector[0], t * axis_unit_vector[1], t * axis_unit_vector[2]]


def _basis_vectors_to_quaternion(x_axis_unit_vector, y_axis_unit_vector, z_axis_unit_vector):
    s = 1.0 + x_axis_unit_vector[0] + y_axis_unit_vector[1] + z_axis_unit_vector[2]
    s = math.sqrt(max(0.0001, s)) / 2.0
    return _normalised_quaternion(numpy.array([
            s,
            (y_axis_unit_vector[2] - z_axis_unit_vector[1]) / (4.0 * s),
            (z_axis_unit_vector[0] - x_axis_unit_vector[2]) / (4.0 * s),
            (x_axis_unit_vector[1] - y_axis_unit_vector[0]) / (4.0 * s)
            ]))


def _quaternion_to_rotation_matrix(q):
    R = numpy.array([
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0]
    ])

    R[0][0] = 1.0 - 2.0 * (q[2] * q[2] + q[3] * q[3])
    R[1][1] = 1.0 - 2.0 * (q[1] * q[1] + q[3] * q[3])
    R[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2])

    a = 2.0 * q[1] * q[2]
    b = 2.0 * q[0] * q[3]
    R[0][1] = a - b
    R[1][0] = a + b

    a = 2.0 * q[1] * q[3]
    b = 2.0 * q[0] * q[2]
    R[0][2] = a + b
    R[2][0] = a - b

    a = 2.0 * q[2] * q[3]
    b = 2.0 * q[0] * q[1]
    R[1][2] = a - b
    R[2][1] = a + b

    return R


def _rotation_matrix_to_quaternion(R):
    return _basis_vectors_to_quaternion([R[0][0], R[1][0], R[2][0]], [R[0][1], R[1][1], R[2][1]], [R[0][2], R[1][2], R[2][2]])


def _from_base_matrix(pos, rot_matrix):
    return numpy.array([
        [rot_matrix[0][0], rot_matrix[0][1], rot_matrix[0][2], pos[0]],
        [rot_matrix[1][0], rot_matrix[1][1], rot_matrix[1][2], pos[1]],
        [rot_matrix[2][0], rot_matrix[2][1], rot_matrix[2][2], pos[2]],
        [0.0,              0.0,              0.0,              1.0   ]
    ])


def _to_base_matrix(pos, rot_matrix):
    p = [
        -(rot_matrix[0][0] * pos[0] + rot_matrix[1][0] * pos[1] + rot_matrix[2][0] * pos[2]),
        -(rot_matrix[0][1] * pos[0] + rot_matrix[1][1] * pos[1] + rot_matrix[2][1] * pos[2]),
        -(rot_matrix[0][2] * pos[0] + rot_matrix[1][2] * pos[1] + rot_matrix[2][2] * pos[2])
    ]
    return numpy.array([
        [rot_matrix[0][0], rot_matrix[1][0], rot_matrix[2][0], p[0]],
        [rot_matrix[0][1], rot_matrix[1][1], rot_matrix[2][1], p[1]],
        [rot_matrix[0][2], rot_matrix[1][2], rot_matrix[2][2], p[2]],
        [0.0,              0.0,              0.0,              1.0 ]
    ])


def _decompose_transform_matrix44(matrix44):
    origin = []
    rotation_matrix = []
    for i in range(3):
        origin.append(matrix44[i][3])
        rotation_matrix.append([])
        for j in range(3):
            rotation_matrix[-1].append(matrix44[i][j])
    return origin, rotation_matrix


def _get_perpendicular_component(decomposed_vector, pivot_vector):
    # result = decomposed_vector + t * pivot_vector
    # result * pivot_vector = 0
    # ---------------------------
    # (decomposed_vector + t * pivot_vector) * pivot_vector = 0
    # t = - (decomposed_vector * pivot_vector) / (pivot_vector * pivot_vector)
    t = -numpy.dot(decomposed_vector, pivot_vector) / numpy.dot(pivot_vector, pivot_vector)
    return numpy.array(decomposed_vector) + t * numpy.array(pivot_vector)


def _angle_between_vectors(u, v):
    denom = _vector_length(u) * _vector_length(v)
    if denom < 0.00001:
        return 0.0
    cos_angle = numpy.dot(u, v) / denom
    if cos_angle <= -1.0:
        return math.pi
    elif cos_angle >= 1.0:
        return 0.0
    return math.acos(cos_angle)


def _computer_rotation_angle(axis_vector, src_vector, dst_vector):
    u = _get_perpendicular_component(src_vector, axis_vector)
    v = _get_perpendicular_component(dst_vector, axis_vector)
    w = numpy.cross(axis_vector, src_vector)
    angle = _angle_between_vectors(u, v)
    if numpy.dot(v, w) < 0.0:
        angle = -angle
    return angle


def _get_bone_joints_in_anim_space(frames_of_bones):
    return [_transform_point(_from_base_matrix(frame["pos"], _quaternion_to_rotation_matrix(frame["rot"])), [0.0, 0.0, 0.0])
            for frame in frames_of_bones]


def _get_bone_joints_in_meta_reference_frame(frames_of_bones, reference_frame):
    F = _to_base_matrix(reference_frame["pos"], _quaternion_to_rotation_matrix(reference_frame["rot"]))
    return [_transform_point(F, p) for p in _get_bone_joints_in_anim_space(frames_of_bones)]


def _compute_weights_of_bones(parents_of_bones, coef=1.5, switch_to_addition=False):
    bone_weights = []
    for i in range(len(parents_of_bones)):
        weight = 1.0
        idx = i
        while parents_of_bones[idx] >= 0:
            if switch_to_addition is False:
                weight *= coef
            else:
                weight += coef
            idx = parents_of_bones[idx]
        bone_weights.append(weight)
    return bone_weights


def _get_keyframes_dir(primary=True, check_exists=True):
    anim_dir = Config.instance.state.anim_dir if primary is True else Config.instance.state.anim_dir2
    if anim_dir is None:
        raise Exception("Cannot load keyframes because the state variable 'anim_dir" +
                        ("" if primary is True else "2") + "' was not set. Please, use the command 'set' first.")
    keyframes_dir = os.path.abspath(os.path.join(Config.instance.state.work_dir, anim_dir))
    if check_exists is True and not os.path.isdir(keyframes_dir):
        raise Exception("Cannot access keyframes directory '" + keyframes_dir + "'. Please, check whether state "
                        "variables 'work_dir' and 'anim_dir" + ("" if primary is True else "2") +
                        "' have proper values so that 'work_dir/anim_dir" + ("" if primary is True else "2") +
                        "' forms the desired disk path.")
    return keyframes_dir


def _get_meta_reference_frames_pathname(primary=True, check_exists=False):
    pathname = os.path.join(_get_keyframes_dir(primary, True), "meta_reference_frames.txt")
    if check_exists is True and not os.path.isfile(pathname):
        raise Exception("The file '" + pathname + "' does not exist. Please, run the command 'reference_frames' first.")
    return pathname


def _is_keyframe_file(pathname):
    return os.path.isfile(pathname) and os.path.basename(pathname).startswith("keyframe") and os.path.basename(pathname).endswith(".txt")


def _get_keyframes_files(primary=True):
    keyframes_dir = _get_keyframes_dir(primary, True)
    pathnames = []
    for fname in os.listdir(keyframes_dir):
        if _is_keyframe_file(os.path.join(keyframes_dir, fname)):
            pathnames.append(os.path.join(keyframes_dir, fname))
    return pathnames


def _load_keyframes(primary=True):
    keyframes = []
    for pathname in _get_keyframes_files(primary):
        with open(pathname, "r") as f:
            lines = f.readlines()
        time_point = float(lines[0])
        num_frames = int(lines[1])
        keyframe = []
        for i in range(num_frames):
            idx = 2+i*7
            pos = [float(lines[idx+0]), float(lines[idx+1]), float(lines[idx+2])]
            idx += 3
            rot = [float(lines[idx+0]), float(lines[idx+1]), float(lines[idx+2]), float(lines[idx+3])]
            keyframe.append({"pos": pos, "rot": rot})
        keyframes.append({"time": time_point, "frames_of_bones": keyframe})
    return sorted(keyframes, key=lambda x: x["time"])


def _transform_one_keyframe_to_world_space(keyframe, parents, pose_frames):
    assert len(keyframe) == len(pose_frames)
    world_matrices = []
    result = []
    for i in range(len(pose_frames)):
        result.append({"pos": _add_vectors(keyframe[i]["pos"], pose_frames[i]["pos"]), "rot": keyframe[i]["rot"]})
        W = _from_base_matrix(result[-1]["pos"], _quaternion_to_rotation_matrix(result[-1]["rot"]))
        if parents[i] >= 0:
            W = _multiply_matrix_by_matrix(world_matrices[parents[i]], W)
            p, R = _decompose_transform_matrix44(W)
            result[-1]["pos"] = p
            result[-1]["rot"] = _normalised_quaternion(_rotation_matrix_to_quaternion(R))
        world_matrices.append(W)
    return result


def _transform_keyframes_to_world_space(keyframes, parents=None, pose_frames=None):
    if parents is None:
        parents = _load_bone_parents()
    if pose_frames is None:
        pose_frames = _load_bone_pose_frames()
    result = []
    for keyframe in keyframes:
        result.append({
            "time": keyframe["time"],
            "frames_of_bones": _transform_one_keyframe_to_world_space(keyframe["frames_of_bones"], parents, pose_frames)
            })
    return result


def _load_meta_reference_frames(primary=True):
    with open(_get_meta_reference_frames_pathname(primary, True), "r") as f:
        lines = f.readlines()
    num_frames = int(lines[0])
    frames = []
    for i in range(num_frames):
        idx = 1+i*7
        pos = [float(lines[idx+0]), float(lines[idx+1]), float(lines[idx+2])]
        idx += 3
        rot = [float(lines[idx+0]), float(lines[idx+1]), float(lines[idx+2]), float(lines[idx+3])]
        frames.append({"pos": pos, "rot": rot})
    return frames


def _load_meta_keyframe_equivalences(pathname):
    with open(pathname, "r") as f:
        raw_lines = f.readlines()
    lines = []
    for raw_line in raw_lines:
        line = raw_line.strip()
        if len(line) > 0 and not line.startswith("%%"):
            lines.append(line)
    assert len(lines) > 1
    result = {}
    anim_name = None
    keyframe_index = -1
    for line in lines[1:]:
        if line[0].isdigit():
            assert anim_name is not None and keyframe_index >= 0 and keyframe_index in result and anim_name in result[keyframe_index]
            result[keyframe_index][anim_name].add(int(line))
        else:
            if line[0] == "@":
                keyframe_index += 1
                result[keyframe_index] = {}
                line = line[1:]
            if len(line) > 0:
                anim_name = line
                assert anim_name not in result[keyframe_index]
                result[keyframe_index][anim_name] = set()
    assert int(lines[0]) - 1 in result
    return result


def _load_bone_parents():
    pathname = os.path.join(os.path.dirname(_get_keyframes_dir()), "parents.txt")
    if not os.path.isfile(pathname):
        raise Exception("Cannot access file '" + pathname + "'.")
    with open(pathname, "r") as f:
        lines = f.readlines()
    if len(lines) == 0:
        raise Exception("Invalid file '" + pathname + "'. At least 1 line must be there.")
    num_bones = int(lines[0])
    if num_bones < 0:
        raise Exception("Invalid file '" + pathname + "'. The number of bones is negative.")
    if num_bones > len(lines):
        raise Exception("Invalid file '" + pathname + "'. The number of bones bigger than number of lines.")
    parents = []
    for i in range(num_bones):
        if len(lines[i+1].strip()) == 0:
            raise Exception("Invalid file '" + pathname + "'. Empty lines are not allowed.")
        bone = int(lines[i+1])
        if bone < -1 or bone >= i:
            raise Exception("Invalid file '" + pathname + "'. Wrong parent bone index at line " + str(i+1) + ".")
        parents.append(bone)
    return parents


def _load_bone_names():
    pathname = os.path.join(os.path.dirname(_get_keyframes_dir()), "names.txt")
    if not os.path.isfile(pathname):
        raise Exception("Cannot access file '" + pathname + "'.")
    with open(pathname, "r") as f:
        lines = f.readlines()
    if len(lines) == 0:
        raise Exception("Invalid file '" + pathname + "'. At least 1 line must be there.")
    num_bones = int(lines[0])
    if num_bones < 0:
        raise Exception("Invalid file '" + pathname + "'. The number of bones is negative.")
    if num_bones > len(lines):
        raise Exception("Invalid file '" + pathname + "'. The number of bones bigger than number of lines.")
    names = []
    for i in range(num_bones):
        if len(lines[i+1].strip()) == 0:
            raise Exception("Invalid file '" + pathname + "'. Empty lines are not allowed.")
        names.append(lines[i+1].strip())
    return names


def _load_bone_pose_frames():
    pathname = os.path.join(Config.instance.state.work_dir, "pose.txt")
    if not os.path.isfile(pathname):
        raise Exception("Cannot access file '" + pathname + "'.")
    with open(pathname, "r") as f:
        lines = f.readlines()
    num_frames = int(lines[0])
    pose_frames = []
    for i in range(num_frames):
        idx = 1+i*7
        pos = [float(lines[idx+0]), float(lines[idx+1]), float(lines[idx+2])]
        idx += 3
        rot = [float(lines[idx+0]), float(lines[idx+1]), float(lines[idx+2]), float(lines[idx+3])]
        pose_frames.append({"pos": pos, "rot": rot})
    return pose_frames


class guarded_actions:
    def __init__(self):
        self.predicates_positive = []
        self.predicates_negative = []
        self.actions = []


def _save_meta_motion_actions(motion_actions):
    with open(os.path.join(_get_keyframes_dir(), "meta_motion_actions.txt"), "w") as f:
        f.write(str(len(motion_actions.keys())) + "\n")
        for keyframe_index in sorted(motion_actions.keys()):
            f.write("%% " + str(keyframe_index) + "\n")
            f.write("@")
            for gactions in motion_actions[keyframe_index]:
                for predicate in gactions.predicates_positive:
                    f.write("[GP]" + predicate[0] + "\n")
                    for param in predicate[1:]:
                        f.write(_float_to_string(param) + "\n")
                for predicate in gactions.predicates_negative:
                    f.write("[GN]" + predicate[0] + "\n")
                    for param in predicate[1:]:
                        f.write(_float_to_string(param) + "\n")
                for predicate in gactions.actions:
                    f.write("[A]" + predicate[0] + "\n")
                    for param in predicate[1:]:
                        f.write(_float_to_string(param) + "\n")


def _load_or_create_meta_motion_actions(num_keyframes):
    pathname = os.path.join(_get_keyframes_dir(), "meta_motion_actions.txt")
    if not os.path.isfile(pathname):
        return {i: [] for i in range(num_keyframes)}
    with open(pathname, "r") as f:
        raw_lines = f.readlines()
    lines = []
    for raw_line in raw_lines:
        line = raw_line.strip()
        if len(line) > 0 and not line.startswith("%%"):
            lines.append(line)
    assert len(lines) > 1
    result = {}
    keyword = None
    keyframe_index = -1
    for line in lines[1:]:
        if line[0].isdigit() or line[0] in ['+', '-']:
            assert keyword is not None and keyframe_index >= 0 and keyframe_index in result and isinstance(result[keyframe_index][-1], guarded_actions)
            if keyword.startswith("[GP]"):
                result[keyframe_index][-1].predicates_positive[-1].append(float(line))
            elif keyword.startswith("[GN]"):
                result[keyframe_index][-1].predicates_negative[-1].append(float(line))
            elif keyword.startswith("[A]"):
                result[keyframe_index][-1].actions[-1].append(float(line))
            else:
                raise Exception("Keyword with unknown classification for keyframe " + str(keyframe_index) + " in file " + pathname)
        else:
            if line[0] == "@":
                keyframe_index += 1
                keyword = None
                result[keyframe_index] = []
                line = line[1:]
            if len(line) > 0:
                if keyword is None or keyword.startswith("[A]") and not line.startswith("[A]"):
                    result[keyframe_index].append(guarded_actions())
                keyword = line
                if keyword.startswith("[GP]"):
                    result[keyframe_index][-1].predicates_positive.append([keyword[4:]])
                elif keyword.startswith("[GN]"):
                    result[keyframe_index][-1].predicates_negative.append([keyword[4:]])
                elif keyword.startswith("[A]"):
                    result[keyframe_index][-1].actions.append([keyword[3:]])
                else:
                    raise Exception("Keyword with unknown classification for keyframe " + str(keyframe_index) + " in file " + pathname)
    assert int(lines[0]) - 1 in result and keyframe_index + 1 == num_keyframes
    return result


def command_colliders_help():
    return """
colliders
    Assigns to each keyframe in the work_dir/anim_dir a collider defined
    by state variable 'shape'. Depending on the value of the variable
    other state variables will be used. Namely:
    * shape is 'capsule':
        - 'length' distance between the half spheres.
        - 'radius' radius of both half spheres.
        The axis of the capsule is parallel to z-axis.
    Each collider is assigned an 'weight', which is always its last value.
    Each collider is assumed to be in the coordinate system of the reference
    frame of the animation (see the command 'reference_frames').
    The computed colliders are always saved into file:
        work_dir/anim_dir/meta_motion_colliders.txt
    If the file exists, them it will be overwritten.
"""


def command_colliders():
    if not os.path.isfile(_get_meta_reference_frames_pathname()):
        raise Exception("The file '" + _get_meta_reference_frames_pathname() + "' does not exist. "
                        "Please, run the command 'reference_frames' first.")
    with open(_get_meta_reference_frames_pathname(), "r") as f:
        num_frames = int(f.readline().strip())
    state = Config.instance.state
    with open(os.path.join(_get_keyframes_dir(), "meta_motion_colliders.txt"), "w") as f:
        f.write(str(num_frames) + "\n")
        for i in range(num_frames):
            f.write("%% " + str(i) + "\n")
            f.write("@" + state.shape + "\n")
            if state.shape == "capsule":
                f.write(_float_to_string(state.length) + "\n")
                f.write(_float_to_string(state.radius) + "\n")
            else:
                raise Exception("Unknown shape '" + state.shape + "' in the state variable 'shape'.")
            f.write(_float_to_string(state.weight) + "\n")


def command_free_bones_help():
    return """
free_bones [look_at]
    TODO!
"""


def command_free_bones():
    if len(Config.instance.cmdline.arguments) != 1:
        raise Exception("Wrong number of arguments.")
    if Config.instance.cmdline.arguments[0] not in ["look_at"]:
        raise Exception("Unknown argument '" + str(Config.instance.cmdline.arguments[0]) + "'.")
    if not os.path.isfile(_get_meta_reference_frames_pathname()):
        raise Exception("The file '" + _get_meta_reference_frames_pathname() + "' does not exist. "
                        "Please, run the command 'reference_frames' first.")
    with open(_get_meta_reference_frames_pathname(), "r") as f:
        num_frames = int(f.readline().strip())
    bone_names = _load_bone_names()
    state = Config.instance.state
    assert all(x in bone_names for x in state.bones.keys())
    with open(os.path.join(_get_keyframes_dir(), "meta_free_bones.txt"), "w") as f:
        f.write(str(num_frames) + "\n")
        for i in range(num_frames):
            f.write("%% " + str(i) + "\n")
            f.write("@" + str(Config.instance.cmdline.arguments[0]) + "\n")
            for bone in sorted(state.bones.keys()):
                f.write("%% " + str(bone) + "\n")
                f.write(str(bone_names.index(bone)) + "\n")
                f.write(str(1 if state.bones[bone] is True else 0) + "\n")


def command_get_help():
    return """
get <var_name>+
    Prints values of given state variables. If no variable name is given, then
    values of all variables are printed.
"""


def command_get():
    var_names = Config.instance.state.vars() if len(Config.instance.cmdline.arguments) == 0 else Config.instance.cmdline.arguments
    if len(var_names) == 1:
        print(str(Config.instance.state.get(var_names[0])))
    else:
        for var in sorted(var_names):
            print(var + ": " + str(Config.instance.state.get(var)))


def command_graph_help():
    return """
graph [pdf]
    Scans all 'meta_keyframe_equivalences.txt' files in animation directories
    under the 'work_dir' and builds Graphviz '.animation_transitions.dot' file,
    representing a graph of transitions between animations. The 'dot' file is
    saved under the 'work_dir'.
    When the output format specifier (i.e. 'pdf') is passed to the command,
    the besides '.animation_transitions.dot' file there is also generated
    '.animation_transitions.pdf' file from the '.animation_transitions.dot'
    file into the same directory as the '.animation_transitions.dot' file.
    NOTE: If any of the output files already exists, then it will be
          overwritten.
    NOTE: Make sure the Graphviz's 'dot' utility is in the PATH environment
          variable, if you pass a format specifier (e.g. pdf) to the command.
"""


def command_graph():
    scan_dirs = [Config.instance.state.work_dir]
    output_dir = Config.instance.state.work_dir

    print("Loading 'meta_keyframe_equivalences.txt' files.")

    equivalences = {}
    for scan_dir in scan_dirs:
        for anim_name in os.listdir(scan_dir):
            pathname = os.path.join(scan_dir, anim_name, 'meta_keyframe_equivalences.txt')
            if anim_name not in equivalences and os.path.isfile(pathname):
                if Config.instance.state.debug_mode is True:
                    print("    Loading: " + pathname)
                equivalences[anim_name] = _load_meta_keyframe_equivalences(pathname)

    dot_pathname = os.path.join(output_dir, ".animation_transitions.dot")
    print("Saving: " + dot_pathname)
    with open(dot_pathname, 'w') as f:
        f.write(
"""
/*
Type this command to console to get PDF of the graph:
    dot -o<output-pathname> -Tpdf .animation_transitions.dot
*/

digraph animation_transitions {
node [shape=box];

"""
        )
        for src_anim_name, src_keyframes in equivalences.items():
            for src_keyframe_index, dst_animations in src_keyframes.items():
                for dst_anim_name, dst_keyframe_indices in dst_animations.items():
                    for dst_keyframe_index in dst_keyframe_indices:
                        if src_anim_name == dst_anim_name and src_keyframe_index < dst_keyframe_index and src_keyframe_index in src_keyframes[dst_keyframe_index][src_anim_name]:
                            continue
                        if dst_keyframe_index == max(equivalences[dst_anim_name].keys()):
                            continue
                        src_label = -1 if src_keyframe_index == max(src_keyframes.keys()) else src_keyframe_index
                        dst_label = dst_keyframe_index
                        if src_label == -1:
                            if dst_label == 0:
                                label = ""
                            else:
                                label = ';' + str(dst_label)
                        else:
                            label = str(src_label) + ';'
                            if dst_label != 0:
                                label += str(dst_label)
                        line = '"' + src_anim_name + '" -> "' + dst_anim_name + '" [label="' + label + '"]'
                        if Config.instance.state.debug_mode is True:
                            print("    " + line)
                        f.write(line + '\n')
        f.write(
"""
}
"""
        )
    if "pdf" in Config.instance.cmdline.arguments:
        pdf_pathname = os.path.splitext(dot_pathname)[0] + ".pdf"
        print("Calling Graphviz's 'dot' utility to generate PDF file:")
        print("    " + os.path.splitext(dot_pathname)[0] + ".pdf")
        subprocess.call(['dot', '-o' + pdf_pathname, '-Tpdf', dot_pathname])
    print("Done.")


def command_help_help():
    return """
help <command>+
    Prints descriptions of all given commands. If no command is given, then
    descriptions of all commands are printed.
"""


def command_help():
    if len(Config.instance.cmdline.arguments) == 0:
        print(_get_script_description())
        print("Here is a list of all available commands:\n")
    for cmd in sorted(Config.instance.commands.keys() if len(Config.instance.cmdline.arguments) == 0 else Config.instance.cmdline.arguments):
        if cmd not in Config.instance.commands:
            raise Exception("Unknown command '" + cmd + "'. Use 'help' command without arguments to see "
                            "the list of all available commands.")
        assert cmd in Config.instance.help_commands
        print(Config.instance.help_commands[cmd]())


def command_joint_distances_help():
    return """
joint_distances [<keyframe-index>|* <keyframe-index>|*] [write|extend|extend!]
    Computes weighted distances between related bone-joints in (different)
    keyframes of (different) animations. The state variables 'anim_dir' and
    'anim_dir2' denote animations, whose keyframes will be considered. The
    first argument is related to 'anim_dir' and the second one to 'anim_dir2'.
    The command can compute joint distances between two concrete keyframes, or
    between a concrete keyframe from one animation with all keyframes of the
    other, or between all keyframe from one animation with all keyframes from
    the other. The special value '*' for an argument allows you to specify
    the use of all keyframes of the corresponding animation.
    The command accepts two kinds of arguments: A keyframe specifier
    '<keyframe-index>|*' or write modifier 'write|extend|extend!'.
    When no keyframe specifier argument is passed or at least one keyframe
    specifier argument is '*', then summary distances (i.e. sum of distances
    between corresponding joint) are computed between keyframes.
    When both keyframe specifier arguments are numbers (i.e. indices of
    concrete keyframes), then there are printed distances per individual
    joint, and also the summary distance.
    When no write modifier is passed, then the results are not written to the
    meta files of the animations. Otherwise, the meta files of the animations,
    namely the files:
        work_dir/anim_dir/meta_keyframe_equivalences.txt
        work_dir/anim_dir2/meta_keyframe_equivalences.txt
    are updated by the computed results. The modifier 'write' replaces results
    of a previous call to this command on the animations by the currently
    computed ones. The modifiers 'extend||extend!' extends the files by the
    currently computed result, i.e. preserving results from the receding calls
    of this command; however, 'extend' version replaces old records colliding
    with the currently computed ones.
    In all cases, the state variable 'delta_time_in_seconds' is used to skip
    equality matching of keyframes whose time from the last considered keyframe
    is smaller than 'delta_time_in_seconds'. So, set the state variable to 0.0,
    if you want all keyframes to be considered.
    NOTE: By 'joint' we actually mean the origin of the coordinate system in
          a keyframe file.
    NOTE: The Euclidean distance between corresponding joints is multiplied by
          weight, equal to number 1.5^<bone_chain>, where <bone_chain> is the
          number of parent bones in the chain from the bone in the joint down
          to a bone without any parent.
    NOTE: Use the command 'list keyframes' to see valid ranges for arguments.
    NOTE: '<keyframe-index>' can be negative, in which case the index is
          assumed in the direction from the last keyframe to the first one.
          Use the symbol '!' instead of minus sign (to prevent confusion
          with dash character for recognition of options). So, for example,
          '!1' represents the number -1 and it refers to the last keyframe;
          '!2' represents the number -2 and it refers to the keyframe preceding
          the last one; and so on.
"""


def command_joint_distances():
    do_write = True if len(Config.instance.cmdline.arguments) > 0 and Config.instance.cmdline.arguments[-1] in ["write", "extend", "extend!"] else False
    do_extend = True if do_write and Config.instance.cmdline.arguments[-1] in ["extend", "extend!"] else False
    do_extend_incremental = True if do_extend is True and Config.instance.cmdline.arguments[-1] == "extend!" else False
    arguments = Config.instance.cmdline.arguments if do_write is False else Config.instance.cmdline.arguments[:-1]
    if "write" in arguments:
        raise Exception("The 'write' argument must be the last argument.")
    if len(arguments) != 0 and len(arguments) != 2:
        raise Exception("Wrong number of argument. Either zero or two keyframe index arguments are expected.")
    if len(arguments) == 0:
        arguments = ["*", "*"]
    for i in range(2):
        try:
            if arguments[i][0] == "!":
                int_value = -int(arguments[i][1:])
                arguments[i] = str(int_value)
            else:
                int(arguments[i])
        except Exception as e:
            if arguments[i] != "*":
                raise Exception("Wrong argument " + str(i+1) + "Expected is either a positive integer, possibly "
                                "prefixed with '!' (to make the number negative), or the symbol '*'.")

    state = Config.instance.state

    parents = _load_bone_parents()
    names = _load_bone_names()
    if len(names) != len(parents):
        raise Exception("Inconsistency between names of bones and parent bone definitions.")
    bone_weights = _compute_weights_of_bones(parents)

    joints_in_reference_frames = {}
    time_points = {}
    for kind, kind_name in [(True, "primary"), (False, "secondary")]:
        keyframes = _transform_keyframes_to_world_space(_load_keyframes(primary=kind), parents)
        reference_frames = _load_meta_reference_frames(primary=kind)
        if len(keyframes) != len(reference_frames):
            raise Exception("Inconsistency between number of keyframes and corresponding meta reference frames "
                            "of the " + kind_name + " animation.")
        joints_in_reference_frames[kind_name] = []
        time_points[kind_name] = {}
        for i in range(len(keyframes)):
            frames = keyframes[i]["frames_of_bones"]
            if len(frames) != len(parents):
                raise Exception("Inconsistency between number of bones in keyframes and parent bone definitions.")
            joints_in_reference_frames[kind_name].append(_get_bone_joints_in_meta_reference_frame(frames, reference_frames[i]))
            time_points[kind_name][i] = keyframes[i]["time"]

    index_filters = {"primary": -1, "secondary": -1}
    for idx, kind_name in [(0, "primary"), (1, "secondary")]:
        if arguments[idx] != "*":
            index_filters[kind_name] = int(arguments[idx])
            if index_filters[kind_name] < 0:
                index_filters[kind_name] += len(joints_in_reference_frames[kind_name])
            if index_filters[kind_name] < 0 or index_filters[kind_name] >= len(joints_in_reference_frames[kind_name]):
                raise Exception("Illegal argument " + str(idx) + ": " + Config.instance.cmdline.arguments[idx] + ". " +
                                "Use the command 'list keyframes' to see valid range for the argument.")

    equality_groups = {"primary": {}, "secondary": {}}
    if len(Config.instance.cmdline.arguments) == 0 or any(index_filters[kind_name] == -1 for kind_name in ["primary", "secondary"]):
        distances = []
        for i in range(len(joints_in_reference_frames["primary"])):
            if index_filters["primary"] > -1 and i != index_filters["primary"]:
                continue
            joints_primary = joints_in_reference_frames["primary"][i]
            for j in range(len(joints_in_reference_frames["secondary"])):
                if state.anim_dir == state.anim_dir2 and j <= i:
                    continue
                if index_filters["secondary"] > -1 and j != index_filters["secondary"]:
                    continue
                joints_secondary = joints_in_reference_frames["secondary"][j]
                if len(joints_primary) != len(joints_secondary):
                    raise Exception("Inconsistency between number of bones in keyframes of primary and secondary animation.")
                distance = sum(_distance_between_points(joints_primary[k], joints_secondary[k]) * bone_weights[k]
                               for k in range(len(joints_primary)) if names[k] in state.bones_filter)
                distances.append([distance, i, j])
        threshold_line_printed = False
        for props in sorted(distances, key=lambda props: -props[0]):
            if threshold_line_printed is False and props[0] <= Config.instance.state.distance_threshold:
                print("----------------------------------------------------")
                threshold_line_printed = True
            print(_float_to_string(props[0]) + "\t" +
                  Config.instance.state.anim_dir + ":" + str(props[1]) + " vs. " +
                  Config.instance.state.anim_dir2 + ":" + str(props[2])
                  )
            if threshold_line_printed is True:
                for idx, kind_name, other_idx in [(1, "primary", 2), (2, "secondary", 1)]:
                    if props[idx] in equality_groups[kind_name]:
                        equality_groups[kind_name][props[idx]].add(props[other_idx])
                    else:
                        equality_groups[kind_name][props[idx]] = {props[other_idx]}
    else:
        distances = {}
        joints_primary = joints_in_reference_frames["primary"][index_filters["primary"]]
        joints_secondary = joints_in_reference_frames["secondary"][index_filters["secondary"]]
        for k in range(len(joints_primary)):
            if names[k] in state.bones_filter:
                distances[names[k]] = _distance_between_points(joints_primary[k], joints_secondary[k]) * bone_weights[k]
        print("=== " + Config.instance.state.anim_dir + ":" + str(index_filters["primary"]) + " vs. " +
              Config.instance.state.anim_dir2 + ":" + str(index_filters["secondary"]) + " ============================")
        for bone_name in sorted(distances.keys(), key=lambda x: -distances[x]):
            print(bone_name + ": " + _float_to_string(distances[bone_name]))
        summary_distance = sum(distances[bone_name] for bone_name in distances.keys())
        print("--------------\nsum: " + _float_to_string(summary_distance))
        if summary_distance <= Config.instance.state.distance_threshold:
            equality_groups["primary"][index_filters["primary"]] = {index_filters["secondary"]}
            equality_groups["secondary"][index_filters["secondary"]] = {index_filters["primary"]}

    for kind_name, other_kind_name in [("primary", "secondary"), ("secondary", "primary")]:
        to_erase = set()
        pivot_idx = None
        for idx in sorted(list(equality_groups[kind_name])):
            if pivot_idx is None or time_points[kind_name][idx] - time_points[kind_name][pivot_idx] >= state.delta_time_in_seconds:
                pivot_idx = idx
            else:
                to_erase.add(idx)
        for idx in to_erase:
            del equality_groups[kind_name][idx]
        other_to_erase = set()
        for idx in equality_groups[other_kind_name]:
            subtraction = equality_groups[other_kind_name][idx] - to_erase
            if len(subtraction) == 0:
                other_to_erase.add(idx)
            else:
                equality_groups[other_kind_name][idx] = subtraction
        for idx in other_to_erase:
            del equality_groups[other_kind_name][idx]

    def transitive_closure(orig_set, map_fwd, map_bwd):
        processed = set()
        closure = orig_set.copy()
        work_list = list(orig_set)
        while len(work_list) > 0:
            elem = work_list[0]
            work_list = work_list[1:]
            closure.add(elem)
            if elem in processed:
                continue
            processed.add(elem)
            if elem in map_bwd:
                for y in map_bwd[elem]:
                    if y in map_fwd:
                        for x in map_fwd[y]:
                            work_list.append(x)
        return closure

    for x in equality_groups["primary"]:
        equality_groups["primary"][x] = transitive_closure(equality_groups["primary"][x], equality_groups["primary"], equality_groups["secondary"])
    for y in equality_groups["secondary"]:
        equality_groups["secondary"][y] = transitive_closure(equality_groups["secondary"][y], equality_groups["secondary"], equality_groups["primary"])

    for kind_name, anim_dir, anim_dir2 in [("primary", state.anim_dir, state.anim_dir2), ("secondary", state.anim_dir2, state.anim_dir)]:
        print("--- " + anim_dir + " -> " + anim_dir2 + " ------------------------------")
        for idx in sorted(list(equality_groups[kind_name])):
            print(str(idx) + " -> " + ", ".join([str(x) for x in sorted(list(equality_groups[kind_name][idx]))]))
        if anim_dir == anim_dir2:
            break

    if do_write is False:
        return

    for kind, kind_name, other_anim_name in [(True, "primary", os.path.basename(state.anim_dir2)),
                                             (False, "secondary", os.path.basename(state.anim_dir))]:

        # First load keyframe equivalences file, if exists.

        keyframe_equivalences = {i: {} for i in range(len(joints_in_reference_frames[kind_name]))}
        pathname = os.path.join(_get_keyframes_dir(kind, True), "meta_keyframe_equivalences.txt")
        if os.path.isfile(pathname):
            with open(pathname, "r") as f:
                lines = f.readlines()
            num_keyframes = int(lines[0])
            bone = -1
            anim_name = None
            for i in range(1, len(lines)):
                line = lines[i].strip()
                if len(line) == 0 or line.startswith("%%"):
                    continue
                if line[0] in ["+", "-"] or line[0].isdigit():
                    if bone not in keyframe_equivalences or anim_name is None or anim_name not in keyframe_equivalences[bone]:
                        raise Exception("A number is not expected at line " + str(i) + " in the file " + pathname)
                    keyframe_equivalences[bone][anim_name].add(int(line))
                else:
                    if line[0] == "@":
                        bone += 1
                        if bone >= num_keyframes:
                            raise Exception("The file '" + pathname + "' contains more records than declared on the first line.")
                        line = line[1:]
                    if len(line) == 0:
                        anim_name = None
                    else:
                        anim_name = line
                        keyframe_equivalences[bone][anim_name] = set()

        # Next write the computed equivalence groups over the loaded keyfame_equivalences.

        if do_extend is False:
            for bone in keyframe_equivalences:
                if other_anim_name in keyframe_equivalences[bone] and (state.anim_dir != state.anim_dir2 or kind is True):
                    del keyframe_equivalences[bone][other_anim_name]
        for idx, group in equality_groups[kind_name].items():
            values = group
            if do_extend_incremental is True and other_anim_name in keyframe_equivalences[idx]:
                values |= keyframe_equivalences[idx][other_anim_name]
            if idx in keyframe_equivalences:
                keyframe_equivalences[idx][other_anim_name] = values
            else:
                keyframe_equivalences[idx] = {other_anim_name: values}

        # Finally, save the updated keyfame_equivalences back to the disk.

        with open(pathname, "w") as f:
            f.write(str(len(keyframe_equivalences)) + "\n")
            for i in range(len(keyframe_equivalences)):
                f.write("%% " + str(i) + "\n@")
                if len(keyframe_equivalences[i]) == 0:
                    f.write("\n")
                    continue
                for anim_name in sorted(list(keyframe_equivalences[i].keys())):
                    f.write(anim_name + "\n")
                    for idx in sorted(list(keyframe_equivalences[i][anim_name])):
                        f.write(str(idx) + "\n")


def command_list_help():
    return """
list [animations|keyframes]
    For the argument 'animations' the command lists animation directories
    under the working directory (see command 'work_dir'), You can use the
    output of this command to set state variables 'anim_dir' and/or
    'anim_dir2'.
    For the argument 'animations' the command prints count of keyframes
    under animation directories defined by variables 'anim_dir' and
    'anim_dir2', in that order.
    When no argument is passed the effect of the command is the same as
    passing the argument 'animations'.
"""


def command_list():
    if len(Config.instance.cmdline.arguments) > 1:
        raise Exception("Wrong number of arguments. At most one argument is expected.")
    if len(Config.instance.cmdline.arguments) == 0 or Config.instance.cmdline.arguments[0] == "animations":
        result = []
        for elem_name in os.listdir(Config.instance.state.work_dir):
            dir_path = os.path.join(Config.instance.state.work_dir, elem_name)
            if not os.path.isdir(dir_path):
                continue
            has_keyframe = False
            for keyframe in os.listdir(dir_path):
                if _is_keyframe_file(os.path.join(dir_path, keyframe)):
                    has_keyframe = True
                    break
            if has_keyframe:
                result.append(elem_name)
        for anim_name in sorted(result):
            print(anim_name)
    elif Config.instance.cmdline.arguments[0] == "keyframes":
        for anim_dir in [Config.instance.state.anim_dir, Config.instance.state.anim_dir2]:
            if anim_dir is None:
                continue
            dir_path = os.path.join(Config.instance.state.work_dir, anim_dir)
            num_keyframes = 0
            for keyframe in os.listdir(dir_path):
                if _is_keyframe_file(os.path.join(dir_path, keyframe)):
                    num_keyframes += 1
            print(anim_dir + ": " + str(num_keyframes))
    else:
        raise Exception("Unknown argument '" + Config.instance.cmdline.arguments[0] + "'.")


def command_mass_distributions_help():
    return """
mass_distributions
    Assigns to each keyframe in the work_dir/anim_dir inverted mass and
    inverted inertia tensor (3x3 matrix) of the rigid body associated
    with the collider (see the command 'colliders') in that keyframe.
    The inverted mass and the inverted inertia tensor are defined by state
    variables 'mass_inverted' and 'inertia_tensor_inverted'. Each inverted
    inertia tensor must be defined in the coordinate system of the reference
    frame of the animation (see the command 'reference_frames').
    The computed values are always saved into file:
        work_dir/anim_dir/meta_mass_distributions.txt
    If the file exists, them it will be overwritten.
"""


def command_mass_distributions():
    if not os.path.isfile(_get_meta_reference_frames_pathname()):
        raise Exception("The file '" + _get_meta_reference_frames_pathname() + "' does not exist. "
                        "Please, run the command 'reference_frames' first.")
    with open(_get_meta_reference_frames_pathname(), "r") as f:
        num_frames = int(f.readline().strip())
    state = Config.instance.state
    with open(os.path.join(_get_keyframes_dir(), "meta_mass_distributions.txt"), "w") as f:
        f.write(str(num_frames) + "\n")
        for i in range(num_frames):
            f.write("%% " + str(i) + "\n")
            f.write("@_\n")
            f.write(_float_to_string(state.mass_inverted) + "\n")
            for i in range(3):
                for j in range(3):
                    f.write(_float_to_string(state.inertia_tensor_inverted[3*i + j]) + "\n")


def command_motion_actions_help():
    return """
motion_actions  <begin> <end>
                set|add|append
                guard|guard_neg|action
                <guard-name>+|<action-name>+
    Updates or creates the motion actions file
        <work_dir>/<anim_dir>/meta_motion_actions.txt
    where <work_dir> and <anim_dir> represent values of the state variables
    'work_dir' and 'anim_dir' respectively. If the file does not exist yet,
    then it is created.
    The numbers <begin> and <end> represent a range of indices of keyframes
    to be considered in the command. Use numbers prefixed with '!' for
    defining the range from the back of the keyframes list.
    The mode 'set'/'add'/'append' replaces/extends/extends the old content
    of the considered keyframes by the list of <guard-name>s or
    <action-name>s passed to the command. The difference between 'add' and
    'append' is that while 'add' extends the last disjunction of guarded
    actions, the 'append' mode appends a new guarded actions to the
    disjunction.
    The specifier 'guard'/'guard_neg' says that next follows a list of
    positive/negative guards. The specifier 'action' says that next follows
    a list of actions.
    Here is a list of supported guards (for <guard-name>s list):
        * 'contact_normal_cone':
            The motion object of the agent have a contact with another object
            and contact lies in the cone.
            Used state variables:
                - 'vec_down_mult * vec_down'
                        defines the axis of cone (unit vector).
                - 'angle'
                        defines a maximal angle between a current normal and
                        the axis of the cone.
        * 'has_any_contact':
            The motion object of the agent have a contact with another object.
            Used state variables: None
        * 'linear_velocity_in_falling_cone':
                - 'angle'
                        defines a maximal angle between current linear
                        velocity of the agent and the axis of the cone,
                        which is the unit vector of the gravitational
                        acceleration. When no gravitational acceleration is
                        acting on the agent then the constraint is trivially
                        false.
                - 'min_linear_speed'
                        if the linear speed in smaller than minimal linear
                        linear speed defined here, then the constraint
                        is trivially false. Otherwise the proper check
                        of the velocity vector in the cone is applied.
        * 'always':
            The guard represents true.
    Here is a list of supported actions (for <action-name>s list):
        * 'none':
            Do nothing, i.e. ignore desired motion of agent's cortex.
            Used state variables: None
        * 'move_forward_with_ideal_speed':
            Applies a force to agent's motion object so that linear velocity
            gets closer to the ideal linear velocity as defined by the
            keyframes.
            Used state variables:
                - 'max_linear_accel'
                        maximal magnitude of the linear acceleration
                - 'motion_error_multiplier'
                        a multiplier (any real number) for the motion error
                        (in this case a difference of ideal and actual speed
                        of the motion object). The motion error has a direct
                        impact on increase/decrease of the animation speed.
        * 'rotate_forward_vector_towards_desired_linear_velocity':
            Rotates the reference frame in the world so that distance between
            the linear velocity and the forward direction is minimal. The
            rotation axis is the up direction of the agent (see the file
            'directions.txt').
            Used state variables:
                - 'max_angular_speed'
                        maximal magnitude of the angular velocity
                - 'max_angular_accel'
                        maximal magnitude of the angular acceleration
                - 'min_linear_speed'
                        is used to compute a multiplier for the
                        angular speed:
                            min(1.0f, max(0.0f, <speed>^2 / min_linear_speed))
                        where '<speed>' is the actual linear speed of the
                        motion object of the agent. The 'min_linear_speed'
                        must be > 0.
        * 'turn_around':
            Rotates the reference frame along the 'up' direction vector by
            angle inferred from the related animation. The goal is to rotate
            agent's forward vector to a desired forward vector (wished by
            the cortex).
            Used state variables:
                - 'max_angular_accel'
                        maximal magnitude of the angular acceleration
        * 'dont_move':
            Creates a linear acceleration on the agent's rigid body so that
            its linear velocity gets smaller and smaller.
            Used state variables:
                - 'max_linear_accel'
                        maximal magnitude of the linear acceleration
                - 'radius'
                        if agent's position in the current simulation step is
                        within a sphere with the center being agent's position
                        in the previous step and radius defined here, then the
                        action makes a force to move the agent to the centre
                        of the sphere. Otherwise, the center of the sphere is
                        reset to the current agent's position and the force
                        acts so that agent's linear speed decreases.
        * 'dont_rotate':
            Creates an angular acceleration on the agent's rigid body so that
            its angular velocity along the given axis (see below) decreases.
            The rotation axis is the up direction of the agent (see the file
            'directions.txt').
            Used state variables:
                - 'max_angular_accel'
                        maximal magnitude of the angular acceleration
        * 'set_linear_velocity':
            Applies a force so that agent's linear velocity becomes the one
            specified in this action.
            Used state variables:
                - 'vec_velocity'
                        A linear velocity to be set to agent. Namely, if
                        'fwd' and 'up' are forward and up direction vectors
                        of the agent in the world space then the velocity
                        to set is the vector:
                            vec_velocity(0) * cross_product(fwd,up) +
                            vec_velocity(1) * fwd +
                            vec_velocity(2) * up.
                - 'max_linear_accel'
                        maximal magnitude of the linear acceleration
        * 'set_angular_velocity':
            Applies a force so that agent's angular velocity becomes the one
            specified in this action.
            Used state variables:
                - 'vec_velocity'
                        A angular velocity to be set to agent. Namely, if
                        'fwd' and 'up' are forward and up direction vectors
                        of the agent in the world space then the velocity
                        to set is the vector:
                            vec_velocity(0) * cross_product(fwd,up) +
                            vec_velocity(1) * fwd +
                            vec_velocity(2) * up.
                - 'max_angular_accel'
                        maximal magnitude of the angular acceleration
        * 'cancel_gravity_accel':
            Applies a force that cancels the effect of the gravitational
            acceleration on the agent.
    NOTE: In the end, each keyframe must have assigned at least one motion
          action.
    NOTE: Guards of all keyframes, which form a branching node in the animation
          graph (due to their equivalence) must capture all possible cases
          (i.e. the disjunction of their guards must form a tautology).
"""


def command_motion_actions():
    if not os.path.isfile(_get_meta_reference_frames_pathname()):
        raise Exception("The file '" + _get_meta_reference_frames_pathname() + "' does not exist. "
                        "Please, run the command 'reference_frames' first.")
    args = Config.instance.cmdline.arguments
    if len(args) < 5:
        raise Exception("Wrong number of arguments. At least 5 arguments are expected.")
    with open(_get_meta_reference_frames_pathname(), "r") as f:
        num_frames = int(f.readline().strip())
    motion_actions = _load_or_create_meta_motion_actions(num_frames)
    state = Config.instance.state
    begin = int(args[0].replace("!", "-"))
    begin = begin if begin >= 0 else num_frames + begin
    end = int(args[1].replace("!", "-"))
    end = end if end >= 0 else num_frames + end + 1
    if end <= begin:
        raise Exception("The begin index of the range of keyframes is not smaller than the end index.")
    for keyframe_index in range(begin, end):
        if args[2] == "set":
            motion_actions[keyframe_index] = [guarded_actions()]
        elif args[2] == "add":
            if len(motion_actions[keyframe_index]) == 0:
                motion_actions[keyframe_index].append(guarded_actions())
        elif args[2] == "append":
            motion_actions[keyframe_index].append(guarded_actions())
        else:
            raise Exception("Wrong argument 3. Expected set or add or append.")

        if args[3] in ["guard", "guard_neg"]:
            if args[3] == "guard":
                constraints = motion_actions[keyframe_index][-1].predicates_positive
            else:
                constraints = motion_actions[keyframe_index][-1].predicates_negative
            for constraint in args[4:]:
                constraints.append([constraint])
                if constraint == "contact_normal_cone":
                    constraints[-1].append(state.vec_down_mult * state.vec_down[0])
                    constraints[-1].append(state.vec_down_mult * state.vec_down[1])
                    constraints[-1].append(state.vec_down_mult * state.vec_down[2])
                    constraints[-1].append(state.angle)
                elif constraint == "has_any_contact":
                    pass    # no arguments
                elif constraint == "linear_velocity_in_falling_cone":
                    constraints[-1].append(state.angle)
                    constraints[-1].append(state.min_linear_speed)
                elif constraint == "always":
                    pass    # no arguments
                else:
                    raise Exception("Unknown constraint name '" + str(constraint) + "'")
        elif args[3] == "action":
            actions = motion_actions[keyframe_index][-1].actions
            for action in args[4:]:
                actions.append([action])
                if action == "none":
                    pass    # no arguments
                elif action == "move_forward_with_ideal_speed":
                    actions[-1].append(state.max_linear_accel)
                    actions[-1].append(state.motion_error_multiplier)
                elif action == "rotate_forward_vector_towards_desired_linear_velocity":
                    actions[-1].append(state.max_angular_speed)
                    actions[-1].append(state.max_angular_accel)
                    actions[-1].append(state.min_linear_speed)
                elif action == "turn_around":
                    actions[-1].append(state.max_angular_accel)
                elif action == "dont_move":
                    actions[-1].append(state.max_linear_accel)
                    actions[-1].append(state.radius)
                elif action == "dont_rotate":
                    actions[-1].append(state.max_angular_accel)
                elif action == "set_linear_velocity":
                    actions[-1].append(state.vec_velocity[0])
                    actions[-1].append(state.vec_velocity[1])
                    actions[-1].append(state.vec_velocity[2])
                    actions[-1].append(state.max_linear_accel)
                elif action == "set_angular_velocity":
                    actions[-1].append(state.vec_velocity[0])
                    actions[-1].append(state.vec_velocity[1])
                    actions[-1].append(state.vec_velocity[2])
                    actions[-1].append(state.max_angular_accel)
                elif action == "cancel_gravity_accel":
                    pass    # no arguments
                else:
                    raise Exception("Unknown action name '" + str(action) + "'")
        else:
            raise Exception("Wrong argument 4. Expected guard or guard_neg or action.")
    _save_meta_motion_actions(motion_actions)


def command_reference_frames_help():
    return """
reference_frames  move_straight|
                  turn_around|
                  turn_around_uniform|
                  all_same_as_first|
                  all_same
    For each keyframe in the work_dir/anim_dir computes a reference frame
    using a procedure identified by the argument. Here is description of
    the available procedures:
    * move_straight:
        for each keyframe the computed reference frame looks as this
            reference_frame {
                "pos": pivot + t * vec_fwd,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
        where t is a solution of equations:
            X = pivot + t * vec_fwd
            vec_fwd * (X - keyframe["frames_of_bones"][bone_idx]["pos"]) = 0
        NOTE: Standing can be considered as a special case of this motion.
    * move_along_vector:
        for each keyframe the computed reference frame looks as this
            reference_frame {
                "pos": pivot + t * vec_transition,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
        where t is a solution of equations:
            X = pivot + t * vec_transition
            vec_transition * (X - keyframe["frames_of_bones"][bone_idx]["pos"]) = 0
        NOTE: Standing can be considered as a special case of this motion.
    * turn_around
        for each keyframe the computed reference frame looks as this
            reference_frame {
                "pos": pivot,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                R * vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            R * vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
        where 'R' is a rotation matrix representing rotation along the axis
        'vec_down_mult * vec_down' by the angle between vectors 'vec_fwd'
        and 'F * T * (vec_fwd, 0)' both projected to the plane perpendicular to
        the axis. The matrix 'T' is the to-base-matrix of
            keyframe_0["frames_of_bones"][bone_idx]
        i.e. of the first keyframe, and 'F' is the from-base-matrix of
            keyframe["frames_of_bones"][bone_idx]
        i.e. of the current keyframe.
    * turn_around_uniform
        for each keyframe the computed reference frame looks as this
            reference_frame {
                "pos": pivot,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                R(i) * vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            R(i) * vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
        where 'R(i) = _quaternion_to_rotation_matrix(
        _axis_angle_to_quaternion(vec_down_mult * vec_down, i * angle /
        <num-keyframes>))', where 'vec_fwd/down_mult', 'vec_fwd/down', and
        'angle' are state variables and the <num-keyframes> is automatically
        inferred from the animation in the 'anim_dir'.
    * all_same_as_first:
        for each keyframe the computed reference frame looks as this:
            reference_frame {
                "pos": pivot + t * vec_fwd,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
        where t is a solution of equations:
            X = pivot + t * vec_fwd
            vec_fwd * (X - keyframe["frames_of_bones"][0]["pos"]) = 0
    * all_same:
        for each keyframe the computed reference frame looks as this:
            reference_frame {
                "pos": pivot,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
    * transition_in_gravity_field
        For each keyframe i the computed reference frame whose origin appears
        on the trajectory:
            X = pivot + t[i] * V + 0.5 * t[i]**2 * G
        where t[i] is the time point of the i-th keyframe, G is the gravity
        acceleration vector (i.e. gravity_accel* vec_down_mult * vec_down),
        and V is an initial velocity at pivot computed as follows:
            V = 1/dt * vec_transition - 0.5 * dt * G
        where dt is the total time of the animation. Note that 'vec_transition'
        is the state variable (just like e.g. pivot or vec_down).
        So, for each keyframe the computed reference frame looks as this:
            reference_frame {
                "pos": pivot + t[i] * V + 0.5 t[i]**2 * G,
                "rot": basis_vectors_to_quaternion(
                            cross_product(
                                vec_fwd_mult * vec_fwd,
                                vec_down_mult * vec_down
                                ),
                            vec_fwd_mult * vec_fwd,
                            vec_down_mult * vec_down
                            )
            }
        NOTE: This procedure is useful for "jump (up/down)" animations.
        NOTE: This procedure also prints the vector V to the console in a form,
              so that you can set it to the state variable 'vec_velocity'
              for the action 'set_linear_velocity'.
    The computed frames (by any procedure) are always saved into file:
        work_dir/anim_dir/meta_reference_frames.txt
    If the file exists, them it will be overwritten.
    NOTE: The command depends on values of these state variables:
          pivot, vec_fwd, vec_fwd_mult, vect_down, vec_down_mult.
"""


def command_reference_frames():
    if len(Config.instance.cmdline.arguments) != 1:
        raise Exception("Wrong number of arguments. Exactly 1 argument is expected.")
    state = Config.instance.state
    meta_reference_frames = []
    keyframes = _transform_keyframes_to_world_space(_load_keyframes())
    if Config.instance.cmdline.arguments[0] == "move_straight":
        motion_direction = numpy.array(state.vec_fwd)
        motion_direction_dot = numpy.dot(motion_direction, motion_direction)
        rot = _basis_vectors_to_quaternion(
                    numpy.cross(state.vec_fwd_mult * motion_direction, state.vec_down_mult * numpy.array(state.vec_down)),
                    state.vec_fwd_mult * motion_direction,
                    state.vec_down_mult * numpy.array(state.vec_down)
                    )
        for keyframe in keyframes:
            t = numpy.dot(motion_direction, numpy.subtract(keyframe["frames_of_bones"][state.bone_idx]["pos"], state.pivot)) / motion_direction_dot
            pos = numpy.add(state.pivot, t * motion_direction)
            meta_reference_frames.append({"pos": pos, "rot": rot})
    elif Config.instance.cmdline.arguments[0] == "move_along_vector":
        motion_direction = numpy.array(state.vec_transition)
        motion_direction_dot = numpy.dot(motion_direction, motion_direction)
        rot = _basis_vectors_to_quaternion(
                    numpy.cross(state.vec_fwd_mult * motion_direction, state.vec_down_mult * numpy.array(state.vec_down)),
                    state.vec_fwd_mult * motion_direction,
                    state.vec_down_mult * numpy.array(state.vec_down)
                    )
        for keyframe in keyframes:
            t = numpy.dot(motion_direction, numpy.subtract(keyframe["frames_of_bones"][state.bone_idx]["pos"], state.pivot)) / motion_direction_dot
            pos = numpy.add(state.pivot, t * motion_direction)
            meta_reference_frames.append({"pos": pos, "rot": rot})
    elif Config.instance.cmdline.arguments[0] == "turn_around":
        rot_axis = _normalised_vector(state.vec_down_mult * numpy.array(state.vec_down))
        T = _to_base_matrix(state.pivot, _quaternion_to_rotation_matrix(keyframes[0]["frames_of_bones"][state.bone_idx]["rot"]))
        vec_fwd_in_bone_space = _transform_vector(T, state.vec_fwd)
        for keyframe in keyframes:
            F = _from_base_matrix(state.pivot, _quaternion_to_rotation_matrix(keyframe["frames_of_bones"][state.bone_idx]["rot"]))
            rotated_vec_fwd = _transform_vector(F, vec_fwd_in_bone_space)
            angle = _computer_rotation_angle(rot_axis, state.vec_fwd, rotated_vec_fwd)
            rot = _normalised_quaternion(_axis_angle_to_quaternion(rot_axis, angle))
            meta_reference_frames.append({"pos": state.pivot, "rot": rot})
    elif Config.instance.cmdline.arguments[0] == "turn_around_uniform":
        rot_axis = _normalised_vector(state.vec_down_mult * numpy.array(state.vec_down))
        uniform_angle = state.angle / (len(keyframes) - 1 if len(keyframes) > 1 else 1)
        for i, keyframe in enumerate(keyframes):
            R = _quaternion_to_rotation_matrix(_axis_angle_to_quaternion(rot_axis, i * uniform_angle))
            rotated_fwd_vector = _normalised_vector(_multiply_matrix_by_vector(R, state.vec_fwd_mult * numpy.array(state.vec_fwd)))
            rot = _basis_vectors_to_quaternion(
                        numpy.cross(rotated_fwd_vector, rot_axis),
                        rotated_fwd_vector,
                        rot_axis
                        )
            meta_reference_frames.append({"pos": state.pivot, "rot": rot})
    elif Config.instance.cmdline.arguments[0] == "all_same_as_first":
        motion_direction = numpy.array(state.vec_fwd)
        rot = _basis_vectors_to_quaternion(
                    numpy.cross(state.vec_fwd_mult * motion_direction, state.vec_down_mult * numpy.array(state.vec_down)),
                    state.vec_fwd_mult * motion_direction,
                    state.vec_down_mult * numpy.array(state.vec_down)
                    )
        t = numpy.dot(motion_direction, numpy.subtract(keyframes[0]["frames_of_bones"][state.bone_idx]["pos"], state.pivot)) / numpy.dot(motion_direction, motion_direction)
        pos = numpy.add(state.pivot, t * motion_direction)
        for _ in range(len(keyframes)):
            meta_reference_frames.append({"pos": pos, "rot": rot})
    elif Config.instance.cmdline.arguments[0] == "all_same":
        motion_direction = numpy.array(state.vec_fwd)
        rot = _basis_vectors_to_quaternion(
                    numpy.cross(state.vec_fwd_mult * motion_direction, state.vec_down_mult * numpy.array(state.vec_down)),
                    state.vec_fwd_mult * motion_direction,
                    state.vec_down_mult * numpy.array(state.vec_down)
                    )
        for _ in range(len(keyframes)):
            meta_reference_frames.append({"pos": state.pivot, "rot": rot})
    elif Config.instance.cmdline.arguments[0] == "transition_in_gravity_field":
        vec_fwd = state.vec_fwd_mult * numpy.array(state.vec_fwd)
        vec_up = state.vec_down_mult * numpy.array(state.vec_down)
        vec_side = _cross_product(vec_fwd, vec_up)
        rot = _basis_vectors_to_quaternion(vec_side, vec_fwd, vec_up)
        dt = keyframes[-1]["time"] - keyframes[0]["time"]
        G = state.gravity_accel * state.vec_down_mult * numpy.array(state.vec_down)
        V = 1/dt * numpy.array(state.vec_transition) - 0.5 * dt * G
        for i in range(len(keyframes)):
            t = keyframes[i]["time"] - keyframes[0]["time"]
            pos = numpy.array(state.pivot) + t * V + 0.5 * t**2 * G
            meta_reference_frames.append({"pos": pos, "rot": rot})
        V[1] = 1/dt * -state.vec_transition[1] - 0.5 * dt * G[1]
        coords = [
            _dot_product(V, vec_side),
            _dot_product(V, vec_fwd),
            _dot_product(V, vec_up),
        ]
        print("Used initial velocity vector 'V' a.k.a. 'vec_velocity':\n[" + ", ".join([_float_to_string(x) for x in coords]) + "]")
    else:
        raise Exception("Unknown argument '" + Config.instance.cmdline.arguments[0] + "'.")
    with open(_get_meta_reference_frames_pathname(), "w") as f:
        f.write(str(len(meta_reference_frames)) + "\n")
        for frame in meta_reference_frames:
            f.write(_float_to_string(frame["pos"][0]) + "\n")
            f.write(_float_to_string(frame["pos"][1]) + "\n")
            f.write(_float_to_string(frame["pos"][2]) + "\n")
            f.write(_float_to_string(frame["rot"][0]) + "\n")
            f.write(_float_to_string(frame["rot"][1]) + "\n")
            f.write(_float_to_string(frame["rot"][2]) + "\n")
            f.write(_float_to_string(frame["rot"][3]) + "\n")


def command_set_help():
    return """
set <var_name> <value>+
    Sets value of a given state variable to the passed value(s).
"""


def command_set():
    if len(Config.instance.cmdline.arguments) < 2:
        raise Exception("Wrong number of arguments. At least 2 arguments are expected.")
    Config.instance.state.set(
            Config.instance.cmdline.arguments[0],
            Config.instance.state.value_from_list_of_strings(Config.instance.cmdline.arguments[0], Config.instance.cmdline.arguments[1:])
            )


def command_plot_origins_help():
    return """
plot_origins
    Prints a table of positions of reference frames and the first bone,
    both in all keyframes. The table is supposed to be copied into
    LibreOffice and used for plots generation.
"""


def command_plot_origins():
    reference_frames = _load_meta_reference_frames(primary=True)
    parents = _load_bone_parents()
    keyframes = _transform_keyframes_to_world_space(_load_keyframes(primary=True), parents)
    print("         reference_frames            keyframes")
    print("time     x        y         z        x        y         z")
    for i, frame in enumerate(keyframes):
        print(
            _float_to_string(frame['time']) + " " +
            _float_to_string(reference_frames[i]["pos"][0]) + " " +
            _float_to_string(reference_frames[i]["pos"][1]) + " " +
            _float_to_string(reference_frames[i]["pos"][2]) + " " +
            _float_to_string(frame["frames_of_bones"][0]["pos"][0]) + " " +
            _float_to_string(frame["frames_of_bones"][0]["pos"][1]) + " " +
            _float_to_string(frame["frames_of_bones"][0]["pos"][2])
            )


def _main():
    if Config.instance.cmdline.command not in Config.instance.commands:
        raise Exception("Unknown command '" + Config.instance.cmdline.command + "'. Use 'help' command to see "
                        "the list of all available commands.")
    Config.instance.commands[Config.instance.cmdline.command]()
    if Config.instance.save_state is True:
        Config.instance.state.save(Config.instance.state_pathname)


if __name__ == "__main__":
    cmdline = None
    try:
        cmdline = _parse_command_line_options()
        commands = {}
        help_commands = {}
        for elem in list(map(eval, dir())):
            if callable(elem) and elem.__name__.startswith("command_"):
                if elem.__name__.endswith("_help") and elem.__name__ != "command_help":
                    help_commands[elem.__name__[len("command_"):-len("_help")]] = elem
                else:
                    commands[elem.__name__[len("command_"):]] = elem
        Config.initialise(cmdline, commands, help_commands)
        _main()
        exit(0)
    except Exception as e:
        sys.stdout.flush()
        print("ERROR: " + str(e))
        sys.stdout.flush()
        if Config.instance is not None and Config.instance.state.debug_mode is True:
            traceback.print_exc()
            sys.stdout.flush()
        exit(1)
