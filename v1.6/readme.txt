writeelf v1.6
by cybermind - cybermind@gmail.com
for Windows
http://cybermind.user.stfunoob.com/wii/writeelf/

Writes a specified ELF file to an SD card for use with the Twilight Hack ( http://wiibrew.org/index.php?title=Twilight_Hack ).

	Usage:
	writeelf [-b[=file]] [-f=file] [-h] [-s] [-v] [-V] drive
	-b          = backup existing data from the SD card
	-b=file     = backup to a specific filename
	-f=file     = file to write to the SD card
	-h          = display this help
	-s=sector   = specify sector to write to (minimum 2048)
	-z          = zero out the ELF area before writing
	-v          = verbose mode (more output)
	-V          = display writeelf version	
	drive       = drive letter of SD card reader device
	
	If -b is specified with no filename, backups will be written
	to incrementally-numbered files named sdbackup##.bak.
	
	The -s option is not recommended unless you know what you are
	doing.


Examples:

To write demo.elf to drive J: and exit:
	writeelf -f=demo.elf J

To backup data from drive H: and exit:
	writeelf -b H

To backup data from drive G: into mybackup.bak and then write demo.elf:
	writeelf -f=demo.elf -b=mybackup.bak G
	
To zero out any previous data and then write demo.elf to drive Q:
	writeelf -f=demo.elf -z Q

To simply test to see if drive I: is accessible:
	writeelf -v I
	


This should not be able to write to a fixed drive, but don't try it anyway!

This will (by design) corrupt any file occupying the 16384 sectors starting
with sector 2048, so just don't leave anything important on the card. You can
also restore the old data by providing a backup filename for the -f parameter.

YOU MUST RUN THIS AS AN ADMINISTRATOR. If you are not logged in as an
administrator, you can use the "runas" command:

	runas /env /user:Administrator "writeelf -f=demo.elf J"
	
where "Administrator" is the administrator account name on your computer. It
will prompt you for a password.



Special thanks to:
 - predakang for MASSIVE information, help and testing on Vista,
 - ruffy91 for bringing up the SP0 drive issue,
 - mattgentl for testing in fixing the SP0 drive issue,
 - #wiidev on EFNet,
 - and last but certainly not least, Team Twiizers for the
   Twilight Hack ( http://www.wiibrew.org/ )
