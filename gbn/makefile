#------------------------------------------------------------------------------#
# File:   makefile				        Fall 2013
# Author: Patrick Vargas			    patrick.vargas@colorado.edu
# University of Colorado Boulder        CSCI 4273: Network Systems
# Description:
#   Compiles the programs found in GBNclient.c and GBNserver.c
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
CC     = gcc
FLAGS  = -Wall
REM    = $(wildcard GBNclient GBNserver *~ *_* *.tar)

all: GBNclient GBNserver

GBNclient: 
	$(CC) $(FLAGS) GBNclient.c -o $@

GBNserver:
	$(CC) $(FLAGS) GBNserver.c -o $@

deploy:
	@mkdir harsha_vargas_gbn;
	@cp GBNclient.c harsha_vargas_gbn;
	@cp GBNserver.c harsha_vargas_gbn;
	@cp makefile harsha_vargas_gbn;
	@cp README.md harsha_vargas_gbn/README.txt;
	@tar -cvf harsha_vargas_gbn.tar harsha_vargas_gbn;

clean:
	@rm $(REM);

# Obligatory unix joke
love:
	@echo not war?
#------------------------------------------------------------------------------#