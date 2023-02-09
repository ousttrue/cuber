glfw_dep = dependency('glfw3', default_options: ['install=true'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
directxmath_dep = dependency('directxmath')
directxtk_dep = dependency('directxtk')
asio_dep = dependency('asio')
spanmath_dep = dependency('spanmath')

bvhutil_lib = static_library(
    'bvhutil',
    [
        'GuiApp.cpp',
        'TurnTable.cpp',
        'Bvh.cpp',
        'BvhSolver.cpp',
        'BvhNode.cpp',
        'Animation.cpp',
        'UdpSender.cpp',
        'BvhPanel.cpp',
        'Payload.cpp',
        'BvhFrame.cpp',
    ],
    dependencies: [
        imgui_dep,
        directxmath_dep,
        directxtk_dep,
        asio_dep,
        spanmath_dep,
    ],
)
bvhutil_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: bvhutil_lib,
    dependencies: [imgui_dep],
)
