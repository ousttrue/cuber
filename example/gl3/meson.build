glfw_dep = dependency('glfw3', default_options: ['install=true'])
c = meson.get_compiler('c')
gl_dep = c.find_library('OpenGL32')

executable(
    'cuber_gl3',
    [
        'main.cpp',
        'GlRenderer.cpp',
        'GlfwPlatform.cpp',
    ],
    install: true,
    dependencies: [
        glfw_dep,
        gl_dep,
        bvhutil_dep,
        cuber_dep,
    ],
)
