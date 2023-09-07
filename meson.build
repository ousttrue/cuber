project(
    'cuber',
    'cpp',
    version: '1.2.0',
    license: 'MIT',
)
# if meson.get_compiler('cpp').get_id() != 'msvc'
#     add_global_arguments('_WIN32_WINNT=0x0602', language: 'cpp')
# endif

subdir('cuber')
if get_option('example')
    subdir('example')
endif
if get_option('tests')
    subdir('tests')
endif
