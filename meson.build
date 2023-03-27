project(
    'cuber',
    'cpp',
    default_options: ['cpp_std=latest'],
    version: '1.0.0',
)

subdir('cuber')
if get_option('example')
    subdir('example')
endif
if get_option('tests')
    subdir('tests')
endif
