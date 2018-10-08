project('miniclassic', 'c')

src = [
    'src/main.c',
    'src/listener.c',
    'src/connection.c',
    'src/rw.c',
    'src/map.c',
    'src/mapgz.c',
    'src/player.c'
]

sdldep = dependency('sdl2', version : '>=2.0.0')
threaddep = dependency('threads')
zlibdep = dependency('zlib')

executable('miniclassic', src, dependencies : [sdldep, threaddep, zlibdep])