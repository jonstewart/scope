#	Copyright 2009, Jon Stewart
#	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.

import os
import glob

def getVar(name):
  try:
    var = os.environ[name]
    return var
  except:
    print(' * You need to set the %s environment variable' % name)
    print('   e.g. export %s=%s' % (name, '/usr/lib/boost'))
    sys.exit()

env = Environment(ENV = os.environ, CPPPATH = [getVar('BOOST_ROOT')], CCFLAGS = '-Wall -Wextra')
env.Command('dummy', env.Program('test', glob.glob('*.cpp')), './$SOURCE')
