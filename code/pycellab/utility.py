import os


def duration_string(start_time, end_time=None, num_decimal_digits=2):
    assert isinstance(start_time, float)
    assert end_time is None or isinstance(end_time, float)
    assert end_time is None or start_time <= end_time or abs(end_time - start_time) < 0.00001
    assert isinstance(num_decimal_digits, int) and num_decimal_digits >= 1
    return format(max(0.0, start_time if end_time is None else end_time - start_time), "." + str(num_decimal_digits) + "f")


def get_progress_string(step, nsteps, num_decimal_digits=1):
    assert isinstance(step, int) and step >= 0
    assert isinstance(nsteps, int) and step < nsteps
    return format(100.0 * step / float(nsteps), "." + str(num_decimal_digits) + "f")


def print_progress_string(step, nsteps, num_decimal_digits=1, shift=4):
    assert isinstance(step, int) and step >= 0
    assert isinstance(nsteps, int) and step < nsteps
    if step % max(1, int(0.001 * nsteps)) == 0:
        print((shift * " ") + get_progress_string(step, nsteps, num_decimal_digits), end='\r')


def disk_path_to_list(path):
    dirname = os.path.dirname(path)
    return ([dirname] if dirname == os.path.dirname(dirname) else disk_path_to_list(dirname)) + [os.path.basename(path)]


def get_common_prefix_of_disk_paths(list_of_paths):
    assert isinstance(list_of_paths, list) and all(isinstance(p, str) for p in list_of_paths)
    if len(list_of_paths) == 0:
        return ""
    if len(list_of_paths) == 1:
        return list_of_paths[0]
    list_of_lists = [disk_path_to_list(os.path.abspath(p)) for p in list_of_paths]
    max_end = min(len(l) for l in list_of_lists)
    end = 0
    while end < max_end:
        are_all_same = True
        for i in range(1, len(list_of_lists)):
            if list_of_lists[i][end] != list_of_lists[0][end]:
                are_all_same = False
                break
        if are_all_same is False:
            break
        end += 1
    result = os.path.join(*list_of_lists[0][:end])
    return result
