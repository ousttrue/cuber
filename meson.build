project(
    'cuber',
    'cpp',
    default_options: ['cpp_std=c++20'],
    version: '1.0.0',
)
subdir('src')
if get_option('example')
    subdir('example')
endif
if get_option('tests')
    subdir('tests')
endif
