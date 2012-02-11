##############################################################################
##                                                                          ##
##  File Name:      BusTest/makefile                                        ##
##                                                                          ##
##  Description:    Builds Linux/Solaris  executable  for  sample  program  ##
##                  BusTest,   which   demonstrates  how  to  utilize  the  ##
##                  services of Intel(R) Quiet System Technology (QST)  to  ##
##                  implement  support  for the enumeration of all devices  ##
##                  connected to the SST/PECI Bus.                          ##
##                                                                          ##
##############################################################################

##############################################################################
##                                                                          ##
##     Copyright (c) 2005-2009, Intel Corporation. All Rights Reserved.     ##
##                                                                          ##
##  Redistribution and use in source and binary  forms,  with  or  without  ##
##  modification, are permitted provided that the following conditions are  ##
##  met:                                                                    ##
##                                                                          ##
##    - Redistributions of source code must  retain  the  above  copyright  ##
##      notice, this list of conditions and the following disclaimer.       ##
##                                                                          ##
##    - Redistributions  in binary form must reproduce the above copyright  ##
##      notice, this list of conditions and the  following  disclaimer  in  ##
##      the   documentation  and/or  other  materials  provided  with  the  ##
##      distribution.                                                       ##
##                                                                          ##
##    - Neither the name  of  Intel  Corporation  nor  the  names  of  its  ##
##      contributors  may  be  used to endorse or promote products derived  ##
##      from this software without specific prior written permission.       ##
##                                                                          ##
##  DISCLAIMER: THIS SOFTWARE IS PROVIDED BY  THE  COPYRIGHT  HOLDERS  AND  ##
##  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  ##
##  BUT  NOT  LIMITED  TO,  THE  IMPLIED WARRANTIES OF MERCHANTABILITY AND  ##
##  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN  NO  EVENT  SHALL  ##
##  INTEL  CORPORATION  OR  THE  CONTRIBUTORS  BE  LIABLE  FOR ANY DIRECT,  ##
##  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  ##
##  (INCLUDING,  BUT  NOT  LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR  ##
##  SERVICES; LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS  INTERRUPTION)  ##
##  HOWEVER  CAUSED  AND  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,  ##
##  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING  ##
##  IN  ANY  WAY  OUT  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  ##
##  POSSIBILITY OF SUCH DAMAGE.                                             ##
##                                                                          ##
##############################################################################

CFLAGS  = -c -fPIC -ggdb -Wno-multichar -I../../Include -I../../Common
LDFLAGS = -ggdb

OS=$(shell uname -o)
ifeq ($(OS),GNU/Linux)
	CC = gcc

	BITS=$(strip $(shell uname -p))
	ifeq ($(BITS),x86_64)
		CFLAGS  += -m64
		LDFLAGS += -m64
	endif
else # Solaris
	CC = /usr/sfw/bin/gcc

	BITS=$(strip $(shell isainfo -b))
	ifeq ($(BITS),64)
		CFLAGS  += -m64
		LDFLAGS += -m64
	endif
endif

##############################################################################
## Commands                                                                 ##
##############################################################################

.PHONY: build
build: Unix/BusTest

.PHONY: clean
clean:
	rm -f -r Unix/*

##############################################################################
## Rules/Dependencies                                                       ##
##############################################################################

Unix:
	mkdir Unix

Unix/BusTest.o: BusTest.c Unix ../../Include/QstCmd.h ../../Include/QstCfg.h \
	../../Include/QstComm.h ../../Include/typedef.h
	$(CC) $(CFLAGS) -o $@ $<

Unix/BusTest: Unix/BusTest.o
	$(CC) $(LDFLAGS) -lQstComm -o $@ $^


