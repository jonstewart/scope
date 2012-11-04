#	Copyright 2009, Jon Stewart
#	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.

import os
import glob

vars = Variables('build_variables.py')
vars.AddVariables(
  ('boostType', 'Suffix to add to Boost libraries to enable finding them', ''),
  ('CC', 'set the name of the C compiler to use (scons finds default)', ''),
  ('CXX', 'set the name of the C++ compiler to use (scons finds default)', ''),
  ('CXXFLAGS', 'add flags for the C++ compiler to CXXFLAGS', '-std=c++11 -stdlib=libc++'),
  ('LINKFLAGS', 'add flags for the linker to LINKFLAGS', '-stdlib=libc++')
)

env = Environment(ENV = os.environ, CCFLAGS = '-Wall -Wextra', variables = vars)
env.Command('dummy', env.Program('test', glob.glob('*.cpp')), './$SOURCE')

vars.Save('build_variables.py', env)
