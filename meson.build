project('miniclassic', 'c')

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
    dependency('threads', required: true),
    dependency('zlib', required: true)
]

if get_option('enable_heartbeat')
    deps += dependency('libcurl', required: true)
    src += 'src/heartbeat.c'

    add_global_arguments('-DENABLE_HEARTBEAT', language: 'c')
endif

configure_file(input: 'settings.ini', output: 'settings.ini', copy: true)

executable('miniclassic', src, dependencies: deps)