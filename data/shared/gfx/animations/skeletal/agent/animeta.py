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
            self.bone_idx = 0
            self.pivot = [0.0, 0.0, 0.83]
            self.shape = "capsule"
            self.length = 0.6
            self.radius = 0.2
            self.vec_down = [0.0, 0.0, -1.0]
            self.vec_fwd = [0.0, -1.0, 0.0]
            self.vec_fwd_end = [0.0, -1.0, 0.0]
            self.vec_down_mult = -1.0
            self.vec_fwd_mult = -1.0
            self.debug_mode = True

        def typeof(self, var_name):
            default_value = Config.State("").get(var_name)
            if default_value is None:
                default_value = ""
            return type(default_value)

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
                return [float(x) for x in list_of_strings]
            else:
                if len(list_of_strings) > 1:
                    raise Exception("Wrong value type. Expected a single value, but passed a list.")
                if var_type == str:
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


def _get_keyframes_dir(check_exists=True):
    if Config.instance.state.anim_dir is None:
        raise Exception("Cannot load keyframes because the state variable 'anim_dir' was not set. Please, use the "
                        "command 'set' first.")
    keyframes_dir = os.path.abspath(os.path.join(Config.instance.state.work_dir, Config.instance.state.anim_dir))
    if check_exists is True and not os.path.isdir(keyframes_dir):
        raise Exception("Cannot access keyframes directory '" + keyframes_dir + "'. Please, check whether state "
                        "variables 'work_dir' and 'anim_dir' have proper values so that 'work_dir/anim_dir' form "
                        "the desired disk path.")
    return keyframes_dir


def _load_keyframes():
    keyframes_dir = _get_keyframes_dir()
    keyframes = []
    for fname in os.listdir(keyframes_dir):
        if fname.startswith("keyframe") and fname.endswith(".txt"):
            with open(os.path.join(keyframes_dir, fname), "r") as f:
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


def command_get_help():
    return """
get <var_name>+
    Prints values of given state variables. If no variable name is given, then
    values of all variables are printed.
"""


def command_get():
    var_names = Config.instance.state.vars() if len(Config.instance.cmdline.arguments) == 0 else Config.instance.cmdline.arguments
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


def command_move_straight_help():
    return """
move_straight
    For each keyframe in the work_dir/anim_dir computes a frame
        frame {
            "pos": pivot + t * vec_fwd
            "rot": basis_vectors_to_quaternion(
                        cross_product(vec_fwd_mult * vec_fwd, vec_down_mult * vec_down),
                        vec_fwd_mult * vec_fwd,
                        vec_down_mult * vec_down
                        )
        }
    where t is computed for each keyframe as a solution of equations:
        X = pivot + t * vec_fwd
        vec_fwd * (X - keyframe["frames_of_bones"][bone_idx]["pos"]) = 0
    The computed frames are then saved into file:
        work_dir/anim_dir/meta_reference_frames.txt
    If the file exists, them it will be overwritten.
    NOTE: The command depends on values of these state variables:
          pivot, vec_fwd, vec_fwd_mult, vect_down, vec_down_mult.
"""


def command_move_straight():
    state = Config.instance.state
    motion_direction = numpy.array(state.vec_fwd)
    motion_direction_dot = numpy.dot(motion_direction, motion_direction)
    rot = _basis_vectors_to_quaternion(
                numpy.cross(state.vec_fwd_mult * motion_direction, state.vec_down_mult * numpy.array(state.vec_down)),
                state.vec_fwd_mult * motion_direction,
                state.vec_down_mult * numpy.array(state.vec_down)
                )
    meta_reference_frames = []
    for keyframe in _load_keyframes():
        t = numpy.dot(motion_direction, numpy.subtract(keyframe["frames_of_bones"][state.bone_idx]["pos"], state.pivot)) / motion_direction_dot
        pos = numpy.add(state.pivot, t * motion_direction)
        meta_reference_frames.append({"pos": pos, "rot": rot})
    with open(os.path.join(_get_keyframes_dir(), "meta_reference_frames.txt"), "w") as f:
        f.write(str(len(meta_reference_frames)) + "\n")
        for frame in meta_reference_frames:
            f.write(str(frame["pos"][0]) + "\n")
            f.write(str(frame["pos"][1]) + "\n")
            f.write(str(frame["pos"][2]) + "\n")
            f.write(str(frame["rot"][0]) + "\n")
            f.write(str(frame["rot"][1]) + "\n")
            f.write(str(frame["rot"][2]) + "\n")
            f.write(str(frame["rot"][3]) + "\n")


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
