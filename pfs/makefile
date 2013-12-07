#------------------------------------------------------------------------------#
# File:   makefile				        Fall 2013
# Author: Patrick Vargas			    patrick.vargas@colorado.edu
# University of Colorado Boulder        CSCI 4273: Network Systems
# Description:
#   Compiles the programs found in udp_client.c and udp_server.c
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
CC     = gcc
FLAGS  = -Wall -lm
REM    = $(wildcard client_PFS server_PFS *~ vargasp_pfs.tar)

all: client_PFS server_PFS

client_PFS: 
	$(CC) $(FLAGS) client_PFS.c -o $@

server_PFS:
	$(CC) $(FLAGS) server_PFS.c -o $@

deploy:
	@mkdir vargasp_pfs;
	@cp client_PFS.c vargasp_pfs;
	@cp server_PFS.c vargasp_pfs;
	@cp makefile vargasp_pfs;
	@cp README.md vargasp_pfs/README.txt;
	@tar -cvf vargasp_pfs.tar vargasp_pfs;
	@rm -r vargasp_pfs

clean:
	@rm $(REM);
	@clear;

# Obligatory unix joke
love:
	@echo not war?
#------------------------------------------------------------------------------#