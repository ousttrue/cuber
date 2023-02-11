glew_dep = dependency('glew')
cuber_gl_inc = include_directories('include')
cuber_gl_lib = static_library(
    'cuber_gl',
    [
        'shader.cpp',
        'GlCubeRenderer.cpp',
        'vao.cpp',
    ],
    include_directories: cuber_gl_inc,
    dependencies: [glew_dep, mesh_dep],
)
cuber_gl_dep = declare_dependency(
    include_directories: [cuber_gl_inc],
    link_with: cuber_gl_lib,
    dependencies: [glew_dep, mesh_dep],
)
