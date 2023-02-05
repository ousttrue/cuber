glfw_dep = dependency('glfw3', default_options: ['install=true'])
c = meson.get_compiler('c')
gl_dep = c.find_library('OpenGL32')
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
directxmath_dep = dependency('directxmath')
directxtk_dep = dependency('directxtk')
asio_dep = dependency('asio')
executable(
    'cuber_sample',
    [
        'main.cpp',
        'GuiWindow.cpp',
        'GuiApp.cpp',
        'GlRenderer.cpp',
        'TurnTable.cpp',
        'Bvh.cpp',
        'BvhSolver.cpp',
        'BvhNode.cpp',
        'Animation.cpp',
    ],
    install: true,
    dependencies: [
        glfw_dep,
        gl_dep,
        imgui_dep,
        glew_dep,
        directxmath_dep,
        directxtk_dep,
        cuber_dep,
        asio_dep,
    ],
)
