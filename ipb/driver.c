/*
 IPB Driver
*/
#include <ntddk.h>
#include <windef.h>
#include "ioctl.h"

void static DriverUnload(IN PDRIVER_OBJECT DriverObject)
{	
	UNICODE_STRING strLink=RTL_CONSTANT_STRING(LINK_NAME);
	IoDeleteSymbolicLink(&strLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

PVOID static GetInputBuffer(IN PIRP Irp)
{
	PIO_STACK_LOCATION irpsp=IoGetCurrentIrpStackLocation(Irp);
	if(irpsp->MajorFunction==IRP_MJ_WRITE)
	{
		PDEVICE_OBJECT devobj=irpsp->DeviceObject;
		if(devobj->Flags & DO_BUFFERED_IO)
			return Irp->AssociatedIrp.SystemBuffer;
		else if(devobj->Flags & DO_DIRECT_IO)
			return MmGetSystemAddressForMdlSafe(Irp->MdlAddress,HighPagePriority);
	}
	else if(irpsp->MajorFunction==IRP_MJ_DEVICE_CONTROL || irpsp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL)
	{
		ULONG CtrlMethod=METHOD_FROM_CTL_CODE(irpsp->Parameters.DeviceIoControl.IoControlCode);
		if(CtrlMethod==METHOD_BUFFERED)
			return Irp->AssociatedIrp.SystemBuffer;
		else if(CtrlMethod==METHOD_IN_DIRECT)
			return MmGetSystemAddressForMdlSafe(Irp->MdlAddress,HighPagePriority);
		else if(CtrlMethod==METHOD_NEITHER)
			return irpsp->Parameters.DeviceIoControl.Type3InputBuffer;
	}
	return NULL;
}

PVOID static GetOutputBuffer(IN PIRP Irp)
{
	PIO_STACK_LOCATION irpsp=IoGetCurrentIrpStackLocation(Irp);
	if(irpsp->MajorFunction==IRP_MJ_READ)
	{
		PDEVICE_OBJECT devobj=irpsp->DeviceObject;
		if(devobj->Flags & DO_BUFFERED_IO)
			return Irp->AssociatedIrp.SystemBuffer;
		else if(devobj->Flags & DO_DIRECT_IO)
			return MmGetSystemAddressForMdlSafe(Irp->MdlAddress,HighPagePriority);
	}
	else if(irpsp->MajorFunction==IRP_MJ_DEVICE_CONTROL || irpsp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL)
	{
		ULONG CtrlMethod=METHOD_FROM_CTL_CODE(irpsp->Parameters.DeviceIoControl.IoControlCode);
		if(CtrlMethod==METHOD_BUFFERED)
			return Irp->AssociatedIrp.SystemBuffer;
		else if(CtrlMethod==METHOD_OUT_DIRECT)
			return MmGetSystemAddressForMdlSafe(Irp->MdlAddress,HighPagePriority);
		else if(CtrlMethod==METHOD_NEITHER)
			return Irp->UserBuffer;
	}
	return NULL;
}

NTSTATUS static DispatchCreateClose(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
	Irp->IoStatus.Status=STATUS_SUCCESS;
	Irp->IoStatus.Information=0;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS static DispatchIoctl(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
	NTSTATUS st=STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION irpsp=IoGetCurrentIrpStackLocation(Irp);
	ULONG IoCtrlCode=irpsp->Parameters.DeviceIoControl.IoControlCode;
	ULONG InputSize=irpsp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG OutputSize=irpsp->Parameters.DeviceIoControl.OutputBufferLength;
	PVOID InputBuffer=GetInputBuffer(Irp);
	PVOID OutputBuffer=GetOutputBuffer(Irp);
	//Start Operation
	switch(IoCtrlCode)
	{
		case IOCTL_Send:
		{
			ZtxInterProcessorBroadcast(InputBuffer);
			st=STATUS_SUCCESS;
			break;
		}
	}
	//End Operation
	if(st==STATUS_SUCCESS)
		Irp->IoStatus.Information=OutputSize;
	else
		Irp->IoStatus.Information=0;
	Irp->IoStatus.Status=st;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return st;
}

void static DriverReinitialize(IN PDRIVER_OBJECT DriverObject,IN PVOID Context OPTIONAL,IN ULONG Count)
{
	ZtxDebugPrint("System CR3: 0x%p\n",(ULONG_PTR)__readcr3());
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING RegistryString)
{
	NTSTATUS st=STATUS_SUCCESS;
	UNICODE_STRING uniDevName=RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING uniLinkName=RTL_CONSTANT_STRING(LINK_NAME);
	PDEVICE_OBJECT pDevObj=NULL;
	//
	DriverObject->MajorFunction[IRP_MJ_CREATE]=DriverObject->MajorFunction[IRP_MJ_CLOSE]=DispatchCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=DispatchIoctl;
	DriverObject->DriverUnload=DriverUnload;
	//
	IoRegisterDriverReinitialization(DriverObject,DriverReinitialize,NULL);
	st=IoCreateDevice(DriverObject,0,&uniDevName,FILE_DEVICE_UNKNOWN,0,FALSE,&pDevObj);
	if(!NT_SUCCESS(st))return st;
	st=IoCreateSymbolicLink(&uniLinkName,&uniDevName);
	if(!NT_SUCCESS(st))IoDeleteDevice(pDevObj);
	return st;
}