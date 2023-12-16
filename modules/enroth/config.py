def can_build(env, platform):
    return True

def configure(env):
    pass

def get_icons_path():
    return "icons"

def get_opts(platform):
    from SCons.Variables import BoolVariable

    return [
        BoolVariable("enroth_generate_types", "Regenerate Enroth types library", False),
    ]
