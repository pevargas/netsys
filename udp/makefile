#------------------------------------------------------------------------------#
# File:   makefile				        Fall 2013
# Author: Patrick Vargas			    patrick.vargas@colorado.edu
# University of Colorado Boulder        CSCI 4273: Network Systems
# Description:
#   Compiles the programs found in udp_client.c and udp_server.c
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
CC     = gcc
FLAGS  = -Wall
REM    = $(wildcard client server *~ vargasp_udp.tar)
REMDIR = $(wildcard vargasp_udp)

all: client server

client: 
	$(CC) $(FLAGS) client.c -o $@

server:
	$(CC) $(FLAGS) server.c -o $@

deploy:
	@mkdir vargasp_udp;
	@cp client.c vargasp_udp;
	@cp server.c vargasp_udp;
	@cp makefile vargasp_udp;
	@cp README.md vargasp_udp/README.txt;
	@tar -cvf vargasp_udp.tar vargasp_udp;

clean:
	@-rm $(REM);
	@-rm -r $(REMDIR);

# Obligatory unix joke
love:
	@echo not war?
#------------------------------------------------------------------------------#