glfw_dep = dependency('glfw3', default_options: ['default_library=static'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])

subdir('bvhutil')
if meson.get_compiler('cpp').get_id() == 'msvc'
    subdir('dx11')
endif
subdir('gl3')
