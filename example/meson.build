glfw_dep = dependency('glfw3', default_options: ['install=true'])
c = meson.get_compiler('c')
gl_dep = c.find_library('OpenGL32')
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
glew_dep = dependency('glew')

executable(
    'cuber_sample',
    [
        'main.cpp',
        'GuiWindow.cpp',
        'GuiApp.cpp',
        'GlRenderer.cpp',
    ],
    install: true,
    dependencies: [
        glfw_dep,
        gl_dep,
        imgui_dep,
        glew_dep,
    ],
)
