#	Copyright 2009, Jon Stewart
#	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.

import os
import glob

def getVar(name, suggested):
  try:
    var = os.environ[name]
    return var
  except:
    print(' * You need to set the %s environment variable' % name)
    print('   e.g. export %s=%s' % (name, suggested))
    sys.exit()

vars = Variables('build_variables.py')
vars.AddVariables(
  ('boostType', 'Suffix to add to Boost libraries to enable finding them', ''),
  ('CC', 'set the name of the C compiler to use (scons finds default)', 'gcc'),
  ('CXX', 'set the name of the C++ compiler to use (scons finds default)', 'g++'),
  ('CXXFLAGS', 'add flags for the C++ compiler to CXXFLAGS', '-std=c++11 -Wall -Wextra'),
  ('CPPPATH', 'Include path for preprocessor', getVar('BOOST_ROOT', '/usr/local/include/boost')),
  ('LINKFLAGS', 'add flags for the linker to LINKFLAGS', '-pthread')
)

env = Environment(ENV = os.environ, variables = vars)
env.Command('dummy', env.Program('test', glob.glob('*.cpp')), './$SOURCE')

vars.Save('build_variables.py', env)
