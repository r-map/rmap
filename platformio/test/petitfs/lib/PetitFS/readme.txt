PetitFS is an Arduino Library wrapper for Petit FatFs:

http://elm-chan.org/fsw/ff/00index_p.html

I started with the files in the avr folder from pfsample.zip.

The pfsample.zip file is in the PetitFS/attic folder.

To see my mods, look at diff_org_mod.txt in the attic folder.

Edit pffArduino.h to set the SD chip select pin and select
an SPI SCK divisor of 2 or 4.

Edit pffconfig.h to select options.  The default is to select
most options.  You can save flash by limiting options.

The PetitFS.ino example uses 5,806 bytes with the default options.

PetitFS.ino uses 4,940 bytes if only _USE_READ and _FS_FAT32 are
selected.

Create TEST.TXT in the root directory of an SD. and run the
PetitFS.ino example.