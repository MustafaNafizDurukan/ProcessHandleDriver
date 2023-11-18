#include <Windows.h>
#include <stdio.h>
#include "..\ProcessHandle\ProcessHandleCommon.h"



int main(int argc, const char* argv[])
{
	printf("IOCTL OPEN Process: (%u)\n", IOCTL_OPEN_PROCESS);

	if (argc != 2) {
		printf("Usage: test <pid>\n");
		return 0;
	}

	HANDLE hDevice = CreateFile(L"\\\\.\\ProcessHandle", GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Error opening device: (%u)", GetLastError());
		return 1;
	}

	ProcessHandleInput input;
	input.ProcessId = atoi(argv[1]);

	ProcessHandleOutput output;
	DWORD bytes;
	BOOL ok = DeviceIoControl(hDevice, IOCTL_OPEN_PROCESS, &input, sizeof(input), &output, sizeof(output), &bytes, nullptr);
	if (!ok) {
		printf("Error: %u", GetLastError());
		return 1;
	}

	printf("Success");
	CloseHandle(hDevice);
}