#include <stdio.h>
#include <Windows.h>
#include <winbase.h>
#include <stdint.h>
#include <strsafe.h>
#include <iostream>
#include <winioctl.h>
#define UNICODE 1
#define _UNICODE 1


/* I love you Mark William Watters, Kerri-Ann Truscott, Tai 
	Truscott, June Truscott & Rodney Watters             */

/* Shout out to National Security Agency Tailored
	Access Operations, e^2m^2 Directorate                */

/* (National Security Agency Equation Group) 		     */

/* Copyright Charles T.W. Truscott, author of this software  */
/* ZeroDisk 0.2, zerodisk.app by Thieving Magpie software    */
/* USE THIS SOFTWARE AT YOUR OWN RISK			     */
/* need to implement EULA, later implementing licensing      */
/* Built for Windows 10					     */
/* Erases 16GB roughly each 29 minutes for a USB thumb drive */

/* FIX: integer overflow each 4.094 gigabytes                */
/* Later implementing passes, e.g. write zeroes, ones and    */
/* Psuedo-random generated numbers 3 times to each byte of   */
/* the drive.						     */

/* As this is my first software, keeping it open source and  */
/* making it a 10yr project from Beta to LTS                 */


/* ZeroDisk 0.2, Authored 20/02/2021 by Charles T.W. Truscott */
/* Beginning my software company thievingmagpie.software      */

/* Last week got my hands on a Google Glass Explorer 2.0      */
/* Eager to flash it with the new Glass OS which avoids       */
/* defunct activation, load up an APK for remote desktop      */
/* protocol and code in Visual Studio via RDP via Google      */
/* Glass						      */

BOOL get_drive_geometry(LPCWSTR path, DISK_GEOMETRY * disk_geometry_structure) {
	printf("Entering function\n");
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	BOOL bResult = FALSE;
	DWORD junk = 0;
	hDevice = CreateFileW(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hDevice == INVALID_HANDLE_VALUE) {
		printf("Error accessing disk\t%d\n", GetLastError());
		return FALSE;
	} else {
		printf("Access disk succeeded\n");
	}
	bResult = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, disk_geometry_structure, sizeof(* disk_geometry_structure), &junk, (LPOVERLAPPED) NULL);
	if(!bResult) {
		printf("%d", GetLastError());
	}
	CloseHandle(hDevice);
	return (bResult);
}


int main(void) {
	DWORD my_drives = 200;
	char drive_strings[200];
	memset(drive_strings, 0, 200);
	DWORD GetDrives = GetLogicalDriveStringsW(my_drives, (LPWSTR) drive_strings);
	int drive_count;
	printf("WELCOME TO ZERODISK. COPYRIGHT CHARLES T.W. TRUSCOTT 2021\n");
	printf("thievingmagpie.software\tzerodisk.app\n\n");
	printf("#################### PLEASE SELECT A DRIVE ####################\n\n");
	printf("   ");
	for(drive_count = 0; drive_count <= 200; ++drive_count){
		printf("%c", drive_strings[drive_count]);
		if(drive_strings[drive_count] == 0x5C) {
			printf("\n\n");
		}
		if(drive_strings[drive_count] == 0x000 && drive_count > 80) {
			break;
		}
	}
	printf("\n");
	printf("###############################################################\n");
	char * drive_letter;
	printf("ENTER DRIVE LETTER (e.g. C, D, E, e.t.c.)\n");
	scanf("%s", drive_letter);
	char path[10];
	strcpy(path, "\\\\.\\");
	strcat(path, (char *) drive_letter);
	strcat(path, ":");
	wchar_t drive_path[10];
	for(int p = 0; p <= 10; ++p) {
		drive_path[p] = (wchar_t) path[p];
		printf("%c", path[p]);
	}
	LPCWSTR drive_path_access = drive_path;
	DISK_GEOMETRY dg = { 0 };
	get_drive_geometry(drive_path_access, &dg);
	wprintf(L"Bytes per sector: %ld\n", dg.BytesPerSector);
	ULONGLONG selected_drive_size = dg.Cylinders.QuadPart * (ULONG) dg.TracksPerCylinder * (ULONG) dg.SectorsPerTrack * (ULONG) dg.BytesPerSector;
	wprintf(L"%I64d bytes\n",  selected_drive_size);
	wprintf(L"%.2f GB\n", (double) selected_drive_size / (1024 * 1024 * 1024));
	Sleep(5000);
	HANDLE AccessDisk = CreateFileW(drive_path_access, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (AccessDisk == INVALID_HANDLE_VALUE) {
		printf("Cannot access disk, quitting\n");
		exit(1);
	}

	int limiting_value = 0;
	LARGE_INTEGER position = { 0 };
	BOOL get_file_pointer = SetFilePointerEx(AccessDisk, position, NULL, FILE_BEGIN);
	printf("\nScanning sector %I64u \n\n", position);
	BYTE read_buffer[65536];
	DWORD read;
	printf("\n############################## BYTES CONTAINED ##############################\n");
	BOOL read_disk = ReadFile(AccessDisk, read_buffer, 65536, &read, NULL);
	int read_limiting_value;
	for(read_limiting_value = 0; read_limiting_value <= 512; ++read_limiting_value) {
		printf("%x", read_buffer[read_limiting_value]);
	}
	BYTE zero_buffer[65537];
	int zero_count;
	for(zero_count = 0; zero_count <= 65536; ++zero_count) {
		BYTE zero_byte = 0;
		zero_buffer[zero_count] = zero_byte;
	}
	DWORD status;
	if(!DeviceIoControl(AccessDisk, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &status, NULL)){
		printf("Dismount failed\n%d", GetLastError());
	}
	int sectors_to_scan;
	printf("%I64d\n", selected_drive_size / 512);
	printf("%I64d\n", selected_drive_size / 512 / 100);
	ULONGLONG sector_count = 0;
	while((ULONGLONG) (sector_count * 65536) <= (ULONGLONG) selected_drive_size){
		LARGE_INTEGER position_again;
		position_again.QuadPart = sector_count * 65536;
		get_file_pointer = SetFilePointerEx(AccessDisk, position_again, NULL, FILE_BEGIN);
		BOOL write_to_disk = WriteFile(AccessDisk, zero_buffer, 65536, NULL, NULL);
		printf("Writing to sector %I64u \t %I64u bytes left\t %I64u bytes written", (ULONGLONG) (position_again.QuadPart), (ULONGLONG) (selected_drive_size) - (ULONGLONG) (position_again.QuadPart), (ULONGLONG) (sector_count * 65536));
		printf("\n");
		printf("#################### %.6lf %c DONE ####################\n", (long double) (100.0 * (((double) (sector_count * 65536)) / (double) (selected_drive_size))), 0x25);
		if (write_to_disk == 0) {
			printf("Writing to sector %I64u failed\n", position);
			printf("%d\n", GetLastError());
		}
		sector_count += 1;
	}
	printf("#################### 100 %c DONE ####################\n", 0x25);
	printf("%I64u BYTES SUCCESSFULLY ERASED\n", (ULONGLONG) (sector_count * 65536)); 
	CloseHandle(AccessDisk);
	return 0;
}