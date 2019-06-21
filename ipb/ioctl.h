#include <ntddk.h>
#include <windef.h>

#define	DEVICE_NAME			L"\\Device\\IPB"
#define LINK_NAME			L"\\DosDevices\\IPB"

#define CTL_CODE_GEN(i)		CTL_CODE(FILE_DEVICE_UNKNOWN, i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_Send			CTL_CODE_GEN(0x801)

void ZtxInterProcessorBroadcast(IN PVOID Context);
void __cdecl ZtxDebugPrint(IN PCSTR Format,...);