glfw_dep = dependency('glfw3', default_options: ['install=true'])
c = meson.get_compiler('c')
gl_dep = c.find_library('OpenGL32')
executable(
    'cuber_sample',
    ['main.cpp'],
    install: true,
    dependencies: [glfw_dep, gl_dep],
)
