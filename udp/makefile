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

clean:
	rm client server *~

# Obligatory unix joke
love:
	@echo not war?
#------------------------------------------------------------------------------#