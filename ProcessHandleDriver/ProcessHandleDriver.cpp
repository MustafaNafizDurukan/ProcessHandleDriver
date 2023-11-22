#include <ntddk.h>
#include "ProcessHandleCommon.h"

void ProcessHandleUnload(PDRIVER_OBJECT);
NTSTATUS ProcessHandleCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcessHandleDeviceControl(PDEVICE_OBJECT, PIRP);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	KdPrint(("ProcessHandle: Driver Entry\n"));
	KdPrint(("Registry path: %wZ\n", RegistryPath));

	DriverObject->DriverUnload = ProcessHandleUnload;

	RTL_OSVERSIONINFOW vi = { sizeof(vi) };
	NTSTATUS status = RtlGetVersion(&vi);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed in RtlGetVersion (0x%X)\n", status));
		return status;
	}

	KdPrint(("Windows version: %u.%u.%u\n", vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber));

	DriverObject->MajorFunction[IRP_MJ_CREATE] = ProcessHandleCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ProcessHandleCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcessHandleDeviceControl;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\ProcessHandle");

	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed in IoCreateDevice (0x%X)\n", status));
		return status;
	}

	DeviceObject->Flags |= DO_BUFFERED_IO;

	UNICODE_STRING symName;
	RtlInitUnicodeString(&symName, L"\\??\\ProcessHandle");

	status = IoCreateSymbolicLink(&symName, &devName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed in IoCreateSymbolicLink (0x%X)\n", status));
		return status;
	}

	return STATUS_SUCCESS;
}

void ProcessHandleUnload(PDRIVER_OBJECT DriverObject) {
	KdPrint(("ProcessHandle: Unload\n"));

	UNICODE_STRING symName;
	RtlInitUnicodeString(&symName, L"\\??\\ProcessHandle");
	IoDeleteSymbolicLink(&symName);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS ProcessHandleDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG len = 0;

	auto& dic = stack->Parameters.DeviceIoControl;
	switch (dic.IoControlCode)
	{
	case IOCTL_OPEN_PROCESS:
		if (dic.Type3InputBuffer == nullptr || Irp->UserBuffer == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		if (dic.InputBufferLength < sizeof(ProcessHandleInput) || dic.OutputBufferLength < sizeof(ProcessHandleInput)) {
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}

		auto input = (ProcessHandleInput*)dic.Type3InputBuffer;
		auto output = (ProcessHandleOutput*)Irp->UserBuffer;

		OBJECT_ATTRIBUTES attr;
		InitializeObjectAttributes(&attr, nullptr, 0, nullptr, nullptr);

		CLIENT_ID cid;
		cid.UniqueProcess = ULongToHandle(input->ProcessId);

		status = ZwOpenProcess(&output->hProcess, PROCESS_ALL_ACCESS, &attr, &cid);
		if (NT_SUCCESS(status)) {
			len = sizeof(output);
		}

		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, 0);

	return status;
}

NTSTATUS ProcessHandleCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);

	return STATUS_SUCCESS;
}