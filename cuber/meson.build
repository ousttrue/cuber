glew_dep = dependency('glew', default_options: ['default_library=static'])
grapho_dep = dependency('grapho')
directxmath_dep = dependency('directxmath')

cuber_inc = include_directories('include')
cuber_lib = static_library(
    'cuber',
    [
        'src/mesh.cpp',
        # 'src/dx/shader.cpp',
        # 'src/dx/DxCubeRenderer.cpp',
        # 'src/dx/DxCubeStereoRenderer.cpp',
        # 'src/dx/DxLineRenderer.cpp',
        'src/gl3/GlCubeRenderer.cpp',
        'src/gl3/GlLineRenderer.cpp',
    ],
    include_directories: cuber_inc,
    dependencies: [glew_dep, grapho_dep, directxmath_dep],
)
cuber_dep = declare_dependency(
    link_with: cuber_lib,
    include_directories: cuber_inc,
    dependencies: [glew_dep, grapho_dep],
)
