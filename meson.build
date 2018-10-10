project('miniclassic', 'c')

cc = meson.get_compiler('c')
cflags = []
is_mingw = (build_machine.system() == 'windows' and cc.get_id() == 'gcc')

src = [
    'src/main.c',
    'src/listener.c',
    'src/connection.c',
    'src/rw.c',
    'src/map.c',
    'src/mapgz.c',
    'src/player.c',
    'src/cpe.c',
    'src/md5.c',
    'src/ini.c',
    'src/config.c'
]

deps = [
    dependency('threads', required: true, static: true),
    dependency('zlib', required: true, static: true)
]

if get_option('enable_heartbeat')
    deps += dependency('libcurl', required: true)
    src += 'src/heartbeat.c'

    add_global_arguments('-DENABLE_HEARTBEAT', language: 'c')
endif

configure_file(input: 'settings.ini', output: 'settings.ini', copy: true)

if build_machine.system() == 'windows'
    deps += cc.find_library('ws2_32')

    src += [
        'src/win32/win_main.c',
        'src/win32/win_err.c'
    ]
elif build_machine.system() == 'linux'
    src += [
        'src/posix/posix_main.c',
        'src/posix/posix_err.c'
    ]
endif

executable('miniclassic', src, dependencies: deps, c_args: cflags)