#------------------------------------------------------------------------------#
# File:   makefile				        Fall 2013
# Author: Patrick Vargas			    patrick.vargas@colorado.edu
# University of Colorado Boulder        CSCI 4273: Network Systems
# Description:
#   
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
CC     = gcc
FLAGS  = -Wall
REM    = $(wildcard routed_LS *~ *.tar *out.txt *log.txt)

all: routed_LS

routed_LS:
	$(CC) $(FLAGS) routed_LS.c -o $@

deploy:
	@mkdir harsha_vargas_tcp;
	@cp routed_LC.c harsha_vargas_tcp;
	@cp makefile harsha_vargas_tcp;
	@cp README.md harsha_vargas_tcp/README.txt;
	@tar -cvf harsha_vargas_tcp.tar harsha_vargas_tcp;

clean:
	@rm $(REM);
	@clear;

# Obligatory unix joke
love:
	@echo not war?
#------------------------------------------------------------------------------#