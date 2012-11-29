#!/usr/bin/env python

import flash
from intelhex import IntelHex
import argparse
import serial_link
import sys
import time

#Command line arguments
parser = argparse.ArgumentParser(description='FPGA Flash Utility')
parser.add_argument('-p', '--port',
                    default=['/dev/ttyUSB0'], nargs=1,
                    help='specify the serial port to use')
parser.add_argument('-c', '--configuration_file',
                    default=['swift-nap_mcs.mcs'], nargs=1,
                    help='hex file to program the flash with')
args = parser.parse_args()
serial_port = args.port[0]
flash_file = args.configuration_file[0]

#Check that ending address of hex file is not in the last sector
#We use the last 16 bytes of the flash for the FPGA DNA hash
ihx = IntelHex(flash_file)
print "Checking to make sure hex file's maximum address is not in last sector"
print "  Maximum addresss =", hex(ihx.maxaddr())
assert ihx.maxaddr() < (flash.FLASHSIZE-flash.SECTORSIZE), "Highest address in hexfile in in last sector"

#Create SerialLink and Flash objects to write to FPGA's flash
print "Creating serial link and adding print callback..."
link = serial_link.SerialLink(port=serial_port)
link.add_callback(serial_link.MSG_PRINT, serial_link.default_print_callback)
print "Creating flash object..."
piksi_flash = flash.Flash(link)
piksi_flash.start()

#Write configuration file to FPGA's flash
print "Writing hex file to FPGA's flash..."
t1 = time.time()
piksi_flash.write_ihx(flash_file)
while bool(piksi_flash._command_queue) or (piksi_flash.flash_operations_pending() > 0):
  time.sleep(0.01)
print "Finished writing hex file to device's flash, took %.2f seconds." % (time.time() - t1)

#Read back the configuration from the flash in order to validate
print "Reading configuration from flash"
t1 = time.time()
piksi_flash.read(0,ihx.maxaddr())
while bool(piksi_flash._command_queue) or (piksi_flash.flash_operations_pending() > 0):
  time.sleep(0.01)
print "Finished reading data from device's flash, took %.2f seconds." % (time.time() - t1)

#Check that the data read back from the flash matches that in the configuration file
piksi_flash.read_cb_sanity_check()
if piksi_flash.rd_cb_data != list(ihx.tobinarray(start=ihx.minaddr(), size=ihx.maxaddr()-ihx.minaddr())):
  raise Exception('Data read from flash does not match configration file')
print "Flashing successful"

#Clean up before exiting
piksi_flash.stop()
link.close()
sys.exit()
