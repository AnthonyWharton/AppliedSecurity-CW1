# Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

modmul : $(wildcard *.[ch])
	@gcc -Wall -std=gnu99 -O3 -o ${@} $(filter %.c, ${^}) -lgmp

.DEFAULT_GOAL = all

all   : modmul test

clean : 
	@rm -f core modmul

STAGES:=1 2 3 4

$(STAGES) :
	@printf "Running Stage $@ ... "
	@./modmul stage$@ < stage$@.input > stage$@.test
	@if diff stage$@.test stage$@.output > /dev/null; then \
		printf "PASSED\n";                             \
	else                                                   \
		printf "FAILED\n";                             \
	fi
	@rm stage$@.test

test : $(STAGES) modmul

