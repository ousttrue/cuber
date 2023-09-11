gtest_dep = dependency('gtest')
gtest_main_dep = dependency('gtest_main')
meshutils_dep = dependency('meshutils')
directxmath_dep = dependency('directxmath')

executable(
    'tests',
    [
        'quat32_test.cpp',
        'ray_test.cpp',
    ],
    install: true,
    dependencies: [
        gtest_dep,
        gtest_main_dep,
        meshutils_dep,
        directxmath_dep,
    ],
)
