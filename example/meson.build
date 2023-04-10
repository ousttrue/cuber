glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])

subdir('bvhutil')
subdir('gl3')
if host_machine.system() == 'windows'
    subdir('dx11')
endif
