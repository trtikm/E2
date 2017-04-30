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
