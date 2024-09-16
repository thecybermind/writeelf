writeelf v1.0
by cybermind - cybermind@gmail.com
for Windows

Writes a specified ELF file to an SD card for use with the Twilight Hack ( http://wiibrew.org/index.php?title=Twilight_Hack ).

Usage:
writeelf <driveletter>: <elffile> [-b]
driveletter = drive letter of the sd card reader
elffile     = path to elf file you wish to write to the sd card
-b          = backup existing data from the write location

If elffile is "-", it will not write anything to the SD card, and will automatically turn on backup mode.

Example:

To only write an ELF file to the drive:
	writeelf j: demo.elf

To only backup data from the drive:
	writeelf j: -
	writeelf j: - -b

To backup data from the drive and then write a new ELF file:
	writeelf j: demo.elf -b


This should not be able to write to a fixed drive, but don't try it anyway!

This will (by design) corrupt any file occupying the 16384 sectors starting
with sector 2048, so just don't leave anything important on the card.
