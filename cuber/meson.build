glew_dep = dependency('glew', default_options: ['default_library=static'])

cuber_inc = include_directories('include')
cuber_lib = static_library(
    'cuber',
    [
        'src/mesh.cpp',
        'src/dx/shader.cpp',
        'src/dx/DxCubeRenderer.cpp',
        'src/dx/DxCubeStereoRenderer.cpp',
        'src/dx/DxLineRenderer.cpp',
        'src/gl3/shader.cpp',
        'src/gl3/vao.cpp',
        'src/gl3/GlCubeRenderer.cpp',
        'src/gl3/GlLineRenderer.cpp',
    ],
    include_directories: cuber_inc,
    dependencies: [glew_dep],
)
cuber_dep = declare_dependency(
    link_with: cuber_lib,
    include_directories: cuber_inc,
    dependencies: [glew_dep],
)
