import os


def compute_root_of_function_using_newton_method(fx, x0, dfdx, maxiters=50000):
    assert callable(fx)
    assert callable(dfdx)
    assert type(x0) in [int, float]
    assert isinstance(maxiters, int) and maxiters > 0
    x = x0
    num_iterations = 0
    while num_iterations < maxiters:
        y = fx(x)
        if abs(y) < 0.001:
            return x
        dy = dfdx(x)
        if abs(dy) < 0.00001:
            raise Exception("Initial value " + str(x0) + " leads to a zero derivative at " + str(x) + ".")
        x -= y/dy
        num_iterations += 1
    raise Exception("No root found within " + str(maxiters) + " iterations.")


def compute_surface_under_function(func, x_lo, x_hi, num_samples=100, delta_x=None):
    assert callable(func)
    assert type(x_lo) in [int, float]
    assert type(x_hi) in [int, float]
    assert x_lo <= x_hi
    assert num_samples is None or (type(num_samples) in [int] and num_samples > 0)
    assert delta_x is None or (type(delta_x) in [int, float] and delta_x > 0)
    assert not((num_samples is None and delta_x is None) or (num_samples is not None and delta_x is not None))
    if delta_x is None:
        delta_x = (x_hi - x_lo) / num_samples
    result = 0.0
    x = x_lo
    while True:
        y = func(x)
        result += y*delta_x
        x += delta_x
        if abs(x - x_hi) < delta_x / 10:
            return result


def merge_dictionaries(left, right):
    result = left.copy()
    result.update(right)
    return result


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
