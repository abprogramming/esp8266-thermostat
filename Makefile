PROGRAM=thermostat
PROGRAM_SRC_DIR=./src
EXTRA_CFLAGS=-DLWIP_HTTPD_CGI=1 -DLWIP_HTTPD_SSI=1 -I./fsdata
EXTRA_COMPONENTS=extras/onewire extras/ds18b20 extras/pwm
EXTRA_COMPONENTS+=extras/mbedtls extras/httpd extras/dhcpserver
include ../../common.mk

html:
	@echo "Generating fsdata.."
	cd fsdata && ./makefsdata
