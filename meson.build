project('obsidian', 'c', version: '0.1a', meson_version: '>=0.47.0')

cc = meson.get_compiler('c')
is_debug = get_option('buildtype').startswith('debug')
ldflags = []
ccflags = []

src = [
    'src/ini.c',
    'src/md5.c',
    'src/xxhash.c',

    'src/commands.c',
    'src/commands_base.c',
    'src/config.c',
    'src/connection.c',
    'src/cpe.c',
    'src/heartbeat.c',
    'src/listener.c',
    'src/main.c',
    'src/map.c',
    'src/mapgen.c',
    'src/mapgen_basic.c',
    'src/mapgen_classic.c',
    'src/mapgz.c',
    'src/mapsave.c',
    'src/namelist.c',
    'src/nbt.c',
    'src/perlin.c',
    'src/player.c',
    'src/rng.c',
    'src/rw.c',
]

deps = [
    dependency('threads', required: true),
    dependency('zlib', required: true)
]

add_global_arguments('-DINI_HANDLER_LINENO', language: 'c')

if get_option('enable_heartbeat')
    deps += dependency('libcurl', required: true)
    src += 'src/heartbeat.c'

    add_global_arguments('-DENABLE_HEARTBEAT', language: 'c')
endif

configure_file(input: 'settings.ini', output: 'settings.ini', copy: true)

version_conf = configuration_data()
version_conf.set('version', meson.project_version())
version_conf.set('compiler_id', cc.get_id())
version_conf.set('compiler_version', cc.version())
version_conf.set('host_os', host_machine.system())
version_conf.set('host_endian', host_machine.endian())
version_conf.set('host_family', host_machine.cpu_family())
configure_file(input: 'version.h.in', output: 'version.h', configuration: version_conf)

builddir_inc = include_directories('.')

if build_machine.system() == 'windows'
    windows = import('windows')
    deps += cc.find_library('ws2_32')
    rc = windows.compile_resources('src/win32/miniclassic.rc', include_directories: builddir_inc)

    src += [
        'src/win32/win_main.c',
        'src/win32/win_err.c',

        rc
    ]
else
    deps += cc.find_library('m')

    src += [
        'src/posix/posix_main.c',
        'src/posix/posix_err.c'
    ]
endif

if host_machine.cpu_family() == 'x86' or host_machine.cpu_family() == 'x86_64' and cc.get_id() == 'gcc'
    ccflags += ['-msse', '-msse2', '-mavx', '-mavx2', '-ftree-vectorize']
endif

executable('obsidian', src, dependencies: deps, include_directories: builddir_inc, link_args: ldflags, c_args: ccflags)
