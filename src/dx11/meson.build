cuber_dx_inc = include_directories('include')
cuber_dx_lib = static_library(
    'cuber_dx',
    [
        'DxCubeRenderer.cpp',
        'DxCubeStereoRenderer.cpp',
        'shader.cpp',
    ],
    include_directories: cuber_dx_inc,
    dependencies: [mesh_dep],
)
cuber_dx_dep = declare_dependency(
    include_directories: cuber_dx_inc,
    link_with: cuber_dx_lib,
    dependencies: [mesh_dep],
)
