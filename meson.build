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

threaddep = dependency('threads')
zlibdep = dependency('zlib')

executable('miniclassic', src, dependencies : [threaddep, zlibdep])