glew_dep = dependency('glew')
cuber_lib = static_library(
    'cuber',
    [
        'CubeRenderer.cpp',
        'LineRenderer.cpp',
    ],
    dependencies: [glew_dep],
)
cuber_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: cuber_lib,
    dependencies: [glew_dep],
)
