cpp = meson.get_compiler('cpp')
winmm_dep = cpp.find_library('winmm')

executable(
    'cuber_dx11',
    [
        'main.cpp',
        'DxPlatform.cpp',
    ],
    install: true,
    dependencies: [
        bvhutil_dep,
        winmm_dep,
        cuber_dep,
    ],
)
