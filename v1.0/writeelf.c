/*
 Code to open SD card reader device is from Costis' SDPatch
 Rest of it (all simple shit) is by cybermind - cybermind@gmail.com
 Run without parameters for usage help
*/

//_snprintf is fine
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VER_WRITEELF "1.0"

#define MAX_ELF_SIZE (1<<23) //8MB
#define SECTOR_SIZE (1<<9) //512KB
#define ELF_WRITE_OFFSET (SECTOR_SIZE*2048) //1MB

unsigned char filebuf[MAX_ELF_SIZE];
unsigned char filebuf2[MAX_ELF_SIZE];

HANDLE NextBackupFile();

int main(int argc, char* argv[]) {
	char drive[3];
	char* elffilename = NULL;
	unsigned long ret;
	char devbuf[MAX_PATH];
	char fulldevname[MAX_PATH];
	HANDLE hdev = NULL;
	HANDLE helf = NULL;
	HANDLE hbak = NULL;
	size_t i;
	unsigned long elfsizelow;
	unsigned long elfsizehigh = 0;
	unsigned long max_write_size;
	int backup = 0;
	int dowrite = 1;

	
	memset(filebuf, 0, sizeof(filebuf));
	memset(filebuf2, 0, sizeof(filebuf2));


	printf("writeelf v%s\n", VER_WRITEELF);
	printf("by cybermind\n");
	printf("cybermind@gmail.com\n\n");


	//small sanity checks
	if (argc < 3) {
		printf("Usage:\n");
		printf("%s <driveletter>: <elffile> [-b]\n", argv[0]);
		printf("driveletter = drive letter of the sd card reader\n");
		printf("elffile     = path to elf file you wish to write to the sd card\n");
		printf("-b          = backup existing data from the write location\n");
		printf("\n");
		printf("If elffile is \"-\", it will not write anything to the SD card,\n");
		printf("and will automatically turn on backup mode. Backup mode will\n");
		printf("store the backed up data in incrementally-numbered files named");
		printf("backup##.bak.\n");
		return 1;
	}
	if (argc >= 4) {
		backup = 1;
	}
	if (!isalpha(argv[1][0])) {
		printf("Drive letter must actually be a letter!\n");
		return 1;
	}
	drive[0] = toupper(argv[1][0]); //toupper not neccesary, just prettier in output
	drive[1] = ':';
	drive[2] = 0;
	elffilename = argv[2];
	if (!strcmp(elffilename, "-") || !strcmp(elffilename, "-b")) {
		dowrite = 0;
		backup = 1;
	}

	
	//fail if drive is not removable
	if (GetDriveType(drive) != DRIVE_REMOVABLE)
	{
		printf("Drive %s is not a removable drive.\n", drive);
		printf("Please specify the drive letter of the SD card reader!\n");
		return 1;
	}
	printf("Drive %s is a removable drive.\n", drive);


	//check if the drive has media in it
	if (!GetVolumeInformation(drive, NULL, 0, NULL, NULL, NULL, NULL, 0)) {
		printf("Drive %s has no media inserted!\n", drive);
		printf("Insert an SD card into reader and try again!\n");
		goto errorcleanup;
	}
	printf("Drive %s has media inserted.\n", drive);

	
	//get device name
	printf("Querying device name of drive %s...", drive);
	ret = QueryDosDevice(drive, devbuf, sizeof(devbuf));
	if (!ret)
	{
		printf("Failed.\n\n");
		printf("Invalid drive letter!\n\n");
		return 1;
	}
	printf("OK!\n");
	// Success! We have retrieved a low-level device name for the specified
	// drive letter. We want to access partition 0, so remove the last part
	// from it which is not needed...
	for (i = strlen(devbuf) - 1; i >= 0; i--)
	{
		if (devbuf[i] == '\\')
		{
			devbuf[i] = 0;
			break;
		}
	}
	// And append a Partition0 indicator to the end of it...
	_snprintf(fulldevname, sizeof(fulldevname), "\\\\.\\GLOBALROOT\\%s\\Partition0", devbuf);
	printf("Device name: %s\n", fulldevname);


	// Create a low-level Windows file to access the drive as a raw device.
	printf("Obtaining low-level access to drive...");
	hdev = CreateFile (fulldevname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
	if (hdev == INVALID_HANDLE_VALUE)
	{
		printf("Failed.\n\n");
		printf("Could not open device!\n\n");
		return 1;
	}
	printf ("OK!\n");

	
	//backup file if we should
	if (backup) {
		printf("Backup mode entered.\n");
		
		//open backup file
		printf("\tOpening file for backup...");
		hbak = NextBackupFile();
		if (!hbak) {
			printf("Failed to open a backup file for writing.\n");
			goto errorcleanup;
		}
		
		
		//read data first time
		printf("\tReading data from device...");
		SetFilePointer(hdev, ELF_WRITE_OFFSET, NULL, FILE_BEGIN);
		ReadFile(hdev, filebuf, sizeof(filebuf), &ret, NULL);
		if (ret != sizeof(filebuf)) {
			printf("Failed.\n\n");
			printf("\tDid not read all data!\n\n");
			goto errorcleanup;
		}
		printf("OK!\n");

		
		//write backup file
		printf("\tWriting backup...");
		i = WriteFile(hbak, filebuf, sizeof(filebuf), &ret, NULL);
		if (!i) {
			printf("Failed.\n\n");
			printf("\tError: %u\n\n", GetLastError());
			goto errorcleanup;
		}
		if (ret != sizeof(filebuf)) {
			printf("Failed.\n\n");
			printf("\tDid not write entire backup file!\n\n");
			goto errorcleanup;
		}
		printf("OK!\n");

		memset(filebuf, 0, sizeof(filebuf));
		CloseHandle(hbak);

		printf("Backup mode completed.\n");
	}


	//end now if we shouldn't write the file ("-" for elffile name)
	if (!dowrite) {
		printf("Writing ELF file to device is disabled, skipping.\n");
		goto errorcleanup;
	}

	//now load the elf file
	printf("Opening ELF file...");
	helf = CreateFile (elffilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (helf == INVALID_HANDLE_VALUE) {
		printf("Failed.\n\n");
		printf("Could not open file \"%s\" for reading!\n\n", elffilename);
		goto errorcleanup;
	}
	printf("OK!\n");


	//get ELF file size, 8MB max
	printf("Getting ELF file size...");
	elfsizelow = GetFileSize(helf, &elfsizehigh);
	if (elfsizelow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
		printf("Failed.\n\n");
		printf("Unable to determine file size of ELF file!\n\n");
		goto errorcleanup;
	}
	if (elfsizehigh || elfsizelow > MAX_ELF_SIZE) {
		printf("Failed.\n\n");
		printf("ELF file is too large!\n");
		printf("Must be under %u bytes.\n", MAX_ELF_SIZE);
		if (elfsizehigh)
			printf("File is %u%.8u bytes.\n\n", elfsizehigh, elfsizelow);
		else
			printf("File is %u bytes.\n\n", elfsizelow);
		goto errorcleanup;
	}
	if (!elfsizehigh && !elfsizelow) {
		printf("Failed.\n\n");
		printf("ELF file is empty!\n\n");
		goto errorcleanup;
	}
	printf("OK!\nELF file size: %u bytes.\n", elfsizelow);


	//max size to write, sector-aligned
	max_write_size = elfsizelow + ((SECTOR_SIZE - (elfsizelow % SECTOR_SIZE)) % SECTOR_SIZE);


	//read entire ELF file into buffer
	printf("Reading ELF file...");
	ReadFile(helf, filebuf, elfsizelow, &ret, NULL);
	if (ret != elfsizelow) {
		printf("Failed.\n\n");
		printf("Did not read entire ELF file!\n\n");
		goto errorcleanup;
	}
	printf("OK!\n");


	//write file to device
	printf("Writing ELF file to device...");
	//move device file pointer to the 1MB mark
	SetFilePointer(hdev, ELF_WRITE_OFFSET, NULL, FILE_BEGIN);
	i = WriteFile(hdev, filebuf, max_write_size, &ret, NULL);
	if (!i) {
		if (GetLastError() == ERROR_MEDIA_CHANGED) {
			printf("Failed.\n\n");
			printf("Error: Your media may have changed!\n\n");
		} else {
			printf("Failed.\n\n");
			printf("Error: %u\n\n", GetLastError());
		}

		goto errorcleanup;
	}
	if (ret != max_write_size) {
		printf("Failed.\n\n");
		printf("Did not write entire ELF file!\n\n");
		goto errorcleanup;
	}
	printf("OK!\n");


	//verify that the data on the card is correct (I guess if it was write-protected it wouldn't work)
	printf("Verifying data on device...");
	SetFilePointer(hdev, ELF_WRITE_OFFSET, NULL, FILE_BEGIN);
	ReadFile(hdev, filebuf2, max_write_size, &ret, NULL);
	if (ret != max_write_size) {
		printf("Failed.\n\n");
		printf("Did not read entire ELF file from device!\n\n");
		goto errorcleanup;
	}
	if (memcmp(filebuf, filebuf2, max_write_size)) {
		printf("Failed.\n\n");
		printf("ELF file does not match!\n\n");
		goto errorcleanup;
	}
	printf("OK!\n");

	printf("\nSuccess!\n\nYour SD card is now patched with \"%s\"!\n", elffilename);


errorcleanup:
	CloseHandle(hdev);
	CloseHandle(helf);

	return 0;
}

//attempts to load an incrementally-numbered backup file
//will return a handle to a newly created file if successful
//will return 0 if it failed to open files 5 times (just in case)
HANDLE NextBackupFile() {
	int i = 0;
	HANDLE x = NULL;
	int attempts  = 0;

	char filename[MAX_PATH];
	for(i = 0; i < 100; i++) {
		_snprintf(filename, sizeof(filename), "sdbackup%.2d.bak", i);
		x = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, 0, 0);
		if (x != INVALID_HANDLE_VALUE) {
			printf("OK!\n", filename);
			printf("\tBackup filename: %s\n", filename);
			return x;
		}

		if (x == INVALID_HANDLE_VALUE) {
			if (GetLastError() == ERROR_FILE_EXISTS)
				continue;
			
			attempts++;
			if (attempts == 6) {
				return 0;
			}
		}
	}

	return 0;
}
