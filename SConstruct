#	Copyright 2009, Jon Stewart
#	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.

import os
import glob

def getVar(name, defaultLoc):
  try:
    var = os.environ[name]
    return var
  except:
    print(' * You need to set the %s environment variable' % name)
    print('   e.g. export %s=%s' % (name, defaultLoc))
    sys.exit()

env = Environment(ENV = os.environ, CPPPATH = [getVar('BOOST_HOME', '~/software/boost_1_38_0')], CCFLAGS = '-Wall -Wextra')
env.Command('dummy', env.Program('test', glob.glob('*.cpp')), './$SOURCE')
