import os

drivers	= [
	'drivers/rs.c',
	'drivers/timer0.c',
	'drivers/timer1.c',
	'drivers/timer2.c',
	'drivers/twi.c',
	'drivers/sleep.c',
	'drivers/spi.c',
	'drivers/eeprom.c',
]

utils	= [
	'utils/fifo.c',
	'utils/time.c',	
	'utils/state_machine.c',
	'utils/majority_voting.c',
]


Import('env')

env.Library('nanoK', drivers + utils)
