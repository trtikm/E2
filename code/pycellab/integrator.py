def euler(dt, variables, derivatives):
    assert dt > 0.0
    assert isinstance(variables, dict)
    assert callable(derivatives)
    df = derivatives(variables)
    assert len(df) == len(variables)
    assert isinstance(df, dict)
    for name in variables.keys():
        assert name in df
        variables[name] += dt * df[name]


def midpoint(dt, variables, derivatives):
    middle_variables = variables.copy()
    euler(dt * 0.5, middle_variables, derivatives)
    df = derivatives(middle_variables)
    def const_middle_derivatives(_): return df
    euler(dt, variables, const_middle_derivatives)
