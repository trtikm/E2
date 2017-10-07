

def duration_string(start_time, end_time=None, num_decimal_digits=2):
    assert isinstance(start_time, float)
    assert end_time is None or isinstance(end_time, float)
    assert end_time is None or start_time <= end_time or abs(end_time - start_time) < 0.00001
    assert isinstance(num_decimal_digits, int) and num_decimal_digits >= 1
    return format(max(0.0, start_time if end_time is None else end_time - start_time), ".2f")
