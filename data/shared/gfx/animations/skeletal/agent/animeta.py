from gettext import translation
import os
import sys
import numpy
import math
import argparse
import json
import traceback


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
            self.distance_threshold = 1.0
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
                    if var_name == "work_dir":
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
    s = math.sqrt(1.0 + x_axis_unit_vector[0] + y_axis_unit_vector[1] + z_axis_unit_vector[2]) / 2.0
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


def command_constraints_help():
    return """
constraints <constraint-type>
    Assigns to each keyframe in the work_dir/anim_dir a constraint defined
    by the passed constraint type. Here are descriptions of available
    constraint types:
    * 'contact_normal_cone':
        - 'vec_down_mult * vec_down' defines the axis of cone (unit vector).
        - 'angle' defines a maximal angle between a current normal and the axis
                  of the cone.
    Each constraint is assumed to be in the coordinate system of the reference
    frame of the animation (see the command 'reference_frames').
    The computed constraints are saved into file:
        work_dir/anim_dir/meta_constraints.txt
    If the file exists, them it will be overwritten.
"""


def command_constraints():
    if not os.path.isfile(_get_meta_reference_frames_pathname()):
        raise Exception("The file '" + _get_meta_reference_frames_pathname() + "' does not exist. "
                        "Please, run the command 'reference_frames' first.")
    if len(Config.instance.cmdline.arguments) != 1:
        raise Exception("Wrong number of argument. A single constraint type must be provided.")
    with open(_get_meta_reference_frames_pathname(), "r") as f:
        num_frames = int(f.readline().strip())
    state = Config.instance.state
    with open(os.path.join(_get_keyframes_dir(), "meta_constraints.txt"), "w") as f:
        f.write(str(num_frames) + "\n")
        for i in range(num_frames):
            f.write("%% " + str(i) + "\n")
            f.write("@" + str(Config.instance.cmdline.arguments[0]) + "\n")
            if Config.instance.cmdline.arguments[0] == "contact_normal_cone":
                f.write(_float_to_string(state.vec_down_mult * state.vec_down[0]) + "\n")
                f.write(_float_to_string(state.vec_down_mult * state.vec_down[1]) + "\n")
                f.write(_float_to_string(state.vec_down_mult * state.vec_down[2]) + "\n")
                f.write(_float_to_string(state.angle) + "\n")
            else:
                raise Exception("Unknown constraint type '" + Config.instance.cmdline.arguments[0] + "'.")


def command_joint_distances_help():
    return """
joint_distances [<keyframe-index>|* <keyframe-index>|*] [write|extend]
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
    '<keyframe-index>|*' or write modifier 'write|extend'.
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
    computed ones. The modifier 'extend' extends the files by the currently
    computed result, i.e. preserving results from the receding calls of this
    command, except that replaced are old records colliding with the currently
    computed ones.
    NOTE: By 'joint' we actually mean the origin of the coordinate system in
          a keyframe file.
    NOTE: The Euclidean distance between corresponding joints is multiplied by
          weight, equal to number 1.5^<bone_chain>, where <bone_chain> is the
          number of parent bones in the chain from the bone in the joint down
          to a bone without any parent.
    NOTE: Use the command 'list keyframes' to see valid ranges for arguments.
"""


def command_joint_distances():
    do_write = True if len(Config.instance.cmdline.arguments) > 0 and Config.instance.cmdline.arguments[-1] in ["write", "extend"] else False
    do_extend = True if do_write and Config.instance.cmdline.arguments[-1] == "extend" else False
    arguments = Config.instance.cmdline.arguments if do_write is False else Config.instance.cmdline.arguments[:-1]
    if "write" in arguments:
        raise Exception("The 'write' argument must be the last argument.")
    if len(arguments) != 0 and len(arguments) != 2:
        raise Exception("Wrong number of argument. Either zero or two keyframe index arguments are expected.")
    if len(arguments) == 0:
        arguments = ["*", "*"]

    state = Config.instance.state

    parents = _load_bone_parents()
    names = _load_bone_names()
    if len(names) != len(parents):
        raise Exception("Inconsistency between names of bones and parent bone definitions.")
    bone_weights = _compute_weights_of_bones(parents)

    joints_in_reference_frames = {}
    for kind, kind_name in [(True, "primary"), (False, "secondary")]:
        keyframes = _load_keyframes(primary=kind)
        reference_frames = _load_meta_reference_frames(primary=kind)
        if len(keyframes) != len(reference_frames):
            raise Exception("Inconsistency between number of keyframes and corresponding meta reference frames "
                            "of the " + kind_name + " animation.")
        joints_in_reference_frames[kind_name] = []
        for i in range(len(keyframes)):
            frames = keyframes[i]["frames_of_bones"]
            if len(frames) != len(parents):
                raise Exception("Inconsistency between number of bones in keyframes and parent bone definitions.")
            joints_in_reference_frames[kind_name].append(_get_bone_joints_in_meta_reference_frame(frames, reference_frames[i]))

    index_filters = {"primary": -1, "secondary": -1}
    for idx, kind_name in [(0, "primary"), (1, "secondary")]:
        if arguments[idx] != "*":
            index_filters[kind_name] = int(arguments[idx])
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
            if do_write is True and threshold_line_printed is True:
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
        for bone_name in sorted(distances.keys(), key=lambda x: -distances[x]):
            print(bone_name + ": " + _float_to_string(distances[bone_name]))
        summary_distance = sum(distances[bone_name] for bone_name in distances.keys())
        print("--------------\nsum: " + _float_to_string(summary_distance))
        if do_write is True and summary_distance <= Config.instance.state.distance_threshold:
            equality_groups["primary"][index_filters["primary"]] = {index_filters["secondary"]}
            equality_groups["secondary"][index_filters["secondary"]] = {index_filters["primary"]}

    if do_write is False:
        return

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
            if idx in keyframe_equivalences:
                keyframe_equivalences[idx][other_anim_name] = group
            else:
                keyframe_equivalences[idx] = {other_anim_name: group}

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
motion_actions <action-name>+
    Assigns to each keyframe in the work_dir/anim_dir passed identifiers of
    motion actions together with their corresponding data. Motion actions
    implement the motion defined by the keyframes. Here are descriptions of
    available actions:
    * 'none':
        Do nothing, i.e. ignore desired motion of agent's cortex.
        This action is useful for 'idle' animations.
    * 'accelerate_towards_clipped_desired_linear_velocity':
        Clips the target linear velocity to the clipping cone and then
        introduces a linear acceleration to get closer to the clipped liner
        velocity. Here are parameters (state variables) of the action:
            - 'vec_fwd' as axis of the clipping cone
            - 'angle' defines a maximal angle between a linear velocity and
                      the axis of the cone.
            - 'min_linear_speed' minimal magnitude of the linear velocity
            - 'max_linear_speed' maximal magnitude of the linear velocity
            - 'max_linear_accel' maximal magnitude of the linear acceleration
    * 'chase_linear_velocity_by_forward_vector':
        Rotates the reference frame in the world so that distance between
        the linear velocity and the forward direction is minimal. Here are
        parameters (state variables) of the action:
            - 'vec_fwd' the forward vector chasing the linear velocity
            - 'vec_down_mult * vec_down' defines rotation axis (unit vector)
            - 'max_angular_speed' maximal magnitude of the angular velocity
            - 'max_angular_accel' maximal magnitude of the angular acceleration
    All motion actions are saved into file:
        work_dir/anim_dir/meta_motion_actions.txt
    If the file exists, them it will be overwritten.
"""


def command_motion_actions():
    if not os.path.isfile(_get_meta_reference_frames_pathname()):
        raise Exception("The file '" + _get_meta_reference_frames_pathname() + "' does not exist. "
                        "Please, run the command 'reference_frames' first.")
    if len(Config.instance.cmdline.arguments) == 0:
        raise Exception("Wrong number of argument. At least one action must be provided.")
    with open(_get_meta_reference_frames_pathname(), "r") as f:
        num_frames = int(f.readline().strip())
    state = Config.instance.state
    with open(os.path.join(_get_keyframes_dir(), "meta_motion_actions.txt"), "w") as f:
        f.write(str(num_frames) + "\n")
        for i in range(num_frames):
            f.write("%% " + str(i) + "\n@")
            for action in Config.instance.cmdline.arguments:
                f.write(str(action) + "\n")
                if action == "none":
                    pass
                elif action == "accelerate_towards_clipped_desired_linear_velocity":
                    f.write(_float_to_string(state.vec_fwd[0]) + "\n")
                    f.write(_float_to_string(state.vec_fwd[1]) + "\n")
                    f.write(_float_to_string(state.vec_fwd[2]) + "\n")
                    f.write(_float_to_string(state.angle) + "\n")
                    f.write(_float_to_string(state.min_linear_speed) + "\n")
                    f.write(_float_to_string(state.max_linear_speed) + "\n")
                    f.write(_float_to_string(state.max_linear_accel) + "\n")
                elif action == "chase_linear_velocity_by_forward_vector":
                    f.write(_float_to_string(state.vec_fwd[0]) + "\n")
                    f.write(_float_to_string(state.vec_fwd[1]) + "\n")
                    f.write(_float_to_string(state.vec_fwd[2]) + "\n")
                    f.write(_float_to_string(state.vec_down_mult * state.vec_down[0]) + "\n")
                    f.write(_float_to_string(state.vec_down_mult * state.vec_down[1]) + "\n")
                    f.write(_float_to_string(state.vec_down_mult * state.vec_down[2]) + "\n")
                    f.write(_float_to_string(state.max_angular_speed) + "\n")
                    f.write(_float_to_string(state.max_angular_accel) + "\n")
                else:
                    raise Exception("Unknown action name '" + str(action) + "'")


def command_reference_frames_help():
    return """
reference_frames  move_straight|
    For each keyframe in the work_dir/anim_dir computes a reference frame
    using a procedure identified by the argument. Here is description of
    the available procedures:
    * move_straight:
        for each keyframe the computed reference frame looks as this
            reference_frame {
                "pos": pivot + t * vec_fwd
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
    if Config.instance.cmdline.arguments[0] == "move_straight":
        motion_direction = numpy.array(state.vec_fwd)
        motion_direction_dot = numpy.dot(motion_direction, motion_direction)
        rot = _basis_vectors_to_quaternion(
                    numpy.cross(state.vec_fwd_mult * motion_direction, state.vec_down_mult * numpy.array(state.vec_down)),
                    state.vec_fwd_mult * motion_direction,
                    state.vec_down_mult * numpy.array(state.vec_down)
                    )
        for keyframe in _load_keyframes():
            t = numpy.dot(motion_direction, numpy.subtract(keyframe["frames_of_bones"][state.bone_idx]["pos"], state.pivot)) / motion_direction_dot
            pos = numpy.add(state.pivot, t * motion_direction)
            meta_reference_frames.append({"pos": pos, "rot": rot})
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
