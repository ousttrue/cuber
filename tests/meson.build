gtest_dep = dependency('gtest')
gtest_main_dep = dependency('gtest_main')
meshutils_dep = dependency('meshutils')
executable(
    'tests',
    ['quat32_test.cpp'],
    dependencies: [
        gtest_dep,
        gtest_main_dep,
        meshutils_dep,
    ],
)
