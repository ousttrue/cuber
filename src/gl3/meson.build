glew_dep = dependency('glew')
cuber_gl3_lib = static_library(
    'cuber_gl3',
    [
        'shader.cpp',
        'GlCubeRenderer.cpp',
        'vao.cpp',
    ],
    dependencies: [glew_dep],
)
cuber_gl3_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: cuber_gl3_lib,
    dependencies: [glew_dep],
)
