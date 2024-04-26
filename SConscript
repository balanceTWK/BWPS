from building import *

cwd     = GetCurrentDir()

src     = Glob('src/*.c')
path    = [cwd + '/inc']

# /* osal */
src     += Glob('port/osal/targets/rt-thread/*.c')
path    += [cwd + '/port/osal/include']
path    += [cwd + '/port/osal/targets/rt-thread']

src     += ['port/bwps_port.c']
path    += [cwd + '/port']

group = DefineGroup('BWPS', src, depend = ['PKG_USING_BWPS'], CPPPATH = path)

Return('group')
