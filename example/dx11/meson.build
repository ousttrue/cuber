cpp = meson.get_compiler('cpp')
winmm_dep = cpp.find_library('winmm')
d3d11_dep = cpp.find_library('d3d11')

executable(
    'cuber_dx11',
    [
        'main.cpp',
        'DxPlatform.cpp',
    ],
    install: true,
    dependencies: [
        bvhutil_dep,
        cuber_dep,
        winmm_dep,
        d3d11_dep,
    ],
)
