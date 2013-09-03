#------------------------------------------------------------------------------#
# File:   makefile				        Fall 2013
# Author: Patrick Vargas			    patrick.vargas@colorado.edu
# University of Colorado Boulder        CSCI 4273: Network Systems
# Description:
#   Compiles the programs found in udp_client.c and udp_server.c
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
CC = gcc
FLAGS = -Wall -O2 

all: client server

client: 
	$(CC) $(FLAGS) udp_client.c -o $@

server:
	$(CC) $(FLAGS) udp_server.c -o $@

deploy:
	@mkdir vargasp_udp;
	@cp udp_client.c vargasp_udp;
	@cp udp_server.c vargasp_udp;
	@cp makefile vargasp_udp;
	@cp README.md vargasp_udp/README.txt;
	@tar -cvf vargasp_udp.tar vargasp_udp;

clean:
	@-rm client server *~ vargasp_udp.tar;
	@-rm -r vargasp_udp;

# Obligatory unix joke
love:
	@echo not war?
#------------------------------------------------------------------------------#