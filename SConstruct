import os

MCU_TARGET	= 'atmega328p'
OPTIMIZE	= '-Os -mcall-prologues -fshort-enums -std=c99 '
includes	= ['.', 'utils', 'drivers', 'externals']
CFLAGS		= '-g -Wall -Wextra ' + OPTIMIZE + '-mmcu=' + MCU_TARGET

env = Environment(
	ENV = os.environ,       \
	CC = 'avr-gcc',		\
	AR = 'avr-ar',		\
	CFLAGS = CFLAGS,	\
	CPPPATH = includes,	\
)

Export('env')

SConscript(['SConscript', ], exports='env')


# suppress reliquat files
env.Alias('clean', '', 'rm -f *~ *o */*.o *.a *.lis')
env.AlwaysBuild('clean')

# display sections size
env.Alias('size', 'libnanoK.a', 'avr-size -t libnanoK.a')
env.AlwaysBuild('size')

# disassemble library
env.Alias('lis', 'libnanoK.a', 'avr-objdump -S -x libnanoK.a > libnanoK.lis')
env.AlwaysBuild('lis')
