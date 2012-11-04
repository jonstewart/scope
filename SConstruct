#	Copyright 2009, Jon Stewart
#	Released under the terms of the Boost license (http://www.boost.org/LICENSE_1_0.txt). See License.txt for details.

import os
import glob

env = Environment(ENV = os.environ, CCFLAGS = '-Wall -Wextra')
env.Command('dummy', env.Program('test', glob.glob('*.cpp')), './$SOURCE')
