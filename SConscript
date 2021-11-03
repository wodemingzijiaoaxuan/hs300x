from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp')
CPPPATH = [cwd + '/']

group = DefineGroup('hs300x', src, depend = ['PKG_USING_HS300X'], CPPPATH = CPPPATH)

Return('group')
