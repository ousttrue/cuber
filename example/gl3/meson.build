glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
c = meson.get_compiler('c')
gl_dep = dependency('OpenGL')

deps = [
    glfw_dep,
    gl_dep,
    bvhutil_dep,
    cuber_dep,
]

if host_machine.system() == 'windows'
    deps += c.find_library('Ws2_32', required: true)
endif

executable(
    'cuber_gl',
    [
        'main.cpp',
        'GlfwPlatform.cpp',
    ],
    install: true,
    dependencies: deps,
)
