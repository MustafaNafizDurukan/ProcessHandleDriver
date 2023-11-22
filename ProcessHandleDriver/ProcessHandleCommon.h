#pragma once

#define		PROCHANDLE					L"ProcessHandle"
#define		PROCHANDLE_SYS_FILE				PROCHANDLE L".sys"

// There are symbols for driver
#define		PROCHANDLE_DEVICENAME_DRV	L"\\Device\\" PROCHANDLE
#define		PROCHANDLE_LINKNAME_DRV 	L"\\??\\" PROCHANDLE

// There are symbols for command line app
#define		PROCHANDLE_LINKNAME_APP 	L"\\\\.\\" PROCHANDLE
#define		PROCHANDLE_SERVNAME_APP	PROCHANDLE

struct ProcessHandleInput
{
	ULONG ProcessId;
};

struct ProcessHandleOutput
{
	HANDLE hProcess;
};

#define PROCHANDLE_DEVICE_IOCTL  0x8301

#define GET_PROCESS_HANDLE_IOCTL  0x820
#define UNKNOWN_FUNC_IOCTL        0x821

#define IOCTL_OPEN_PROCESS CTL_CODE(PROCHANDLE_DEVICE_IOCTL, GET_PROCESS_HANDLE_IOCTL, METHOD_BUFFERED, FILE_ANY_ACCESS)