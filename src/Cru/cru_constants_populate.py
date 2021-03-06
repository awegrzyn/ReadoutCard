import fileinput
import re
import cru_table as table
import os

#'':'NUM_DROPPED_PACKETS'
roc_regs = {'add_bsp_hkeeping_tempstat':'TEMPERATURE',
'add_bsp_info_builddate':'FIRMWARE_DATE',
'add_bsp_info_buildtime':'FIRMWARE_TIME',
'add_bsp_hkeeping_chipid_low':'FPGA_CHIP_LOW',
'add_bsp_hkeeping_chipid_high':'FPGA_CHIP_HIGH',
'add_ttc_clkgen_ttc240freq':'CTP_CLOCK',
'add_ttc_clkgen_lcl240freq':'LOCAL_CLOCK',
'add_gbt_wrapper0':'WRAPPER0',
'add_gbt_wrapper1':'WRAPPER1'}


# e.g. 'TEMPERATURE':0x00010008
to_replace = {}

for key0,value0 in roc_regs.iteritems():
  for key,value in table.CRUADD.iteritems():
    if (key0 == key):
      to_replace[value0] = '0x' + str(format(value, '08x'))

print to_replace 

cfile = open('Constants.h')
contents = cfile.readlines()

for key,value in to_replace.iteritems():
  for (i, line) in enumerate(contents):
    if (key in line):
      contents[i] = re.sub("\([^)]*\)", '(' + value + ')', line)

cfile = open('Constants.h', 'w')
cfile.writelines(contents)
