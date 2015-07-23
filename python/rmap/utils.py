
def nint(x):
    """iround(number) -> integer
    Round a number to the nearest integer."""

    y = round(x) - .5
    return int(y) + (y > 0)

