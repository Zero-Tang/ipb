#include <ntddk.h>
#include <windef.h>

/*
  Introduction to IPB designation:

  We use Queued DPC as Inter-Processor Broadcast media.
  Note that we should not use KeGenericDpcCall because this
  function is only available and exported on Windows NT6.
  We use KeInsertQueueDpc to broadcast DPC to all CPU cores,
  for it is compactible to Windows Server 2000, XP and 2003.

  If there is only one processor, simply raise IRQL to DPC
  level and invoke the DPC Routine, then lower the IRQL back.

  This designation allows 256 CPU cores at most in theory.
  However, I haven't tested if this is true.
*/

void static ZtxDpcRT(IN PKDPC Dpc,IN PVOID DeferedContext OPTIONAL,IN PVOID SystemArgument1 OPTIONAL,IN PVOID SystemArgument2 OPTIONAL)
{
	PLONG volatile n=(PLONG)SystemArgument1;
	DbgPrint("[IPB] This is broadcast call in processor %d\n",KeGetCurrentProcessorNumber());
	//Decrement the GlobalOperatingNumber by 1 as we completed operation.
	InterlockedDecrement(n);
}

void ZtxInterProcessorBroadcast(IN PVOID Context)
{
	if(KeNumberProcessors==1)
	{
		KIRQL OldIrql=KeRaiseIrqlToDpcLevel();
		LONG volatile n=1;
		ZtxDpcRT(NULL,Context,(PVOID)&n,NULL);
		KeLowerIrql(OldIrql);
	}
	else
	{
		PVOID IpbBuffer=ExAllocatePool(NonPagedPool,sizeof(KDPC)*KeNumberProcessors+4);
		if(IpbBuffer)
		{
			BYTE i=0;
			PKDPC pDpc=(PKDPC)IpbBuffer;
			PLONG volatile GlobalOperatingNumber=(PLONG)((ULONG_PTR)IpbBuffer+sizeof(KDPC)*KeNumberProcessors);
			DbgPrint("[IPB] IPB Broadcast is started!\n");
			RtlZeroMemory(pDpc,sizeof(KDPC)*KeNumberProcessors);
			//The "Global Operating Number" is a flag for waiting all processors to complete operation.
			*GlobalOperatingNumber=KeNumberProcessors;
			for(;i<KeNumberProcessors;i++)
			{
				KeInitializeDpc(&pDpc[i],ZtxDpcRT,Context);
				KeSetTargetProcessorDpc(&pDpc[i],i);
				KeSetImportanceDpc(&pDpc[i],HighImportance);
				KeInsertQueueDpc(&pDpc[i],(PVOID)GlobalOperatingNumber,NULL);
			}
			//When GlobalOperatingNumber=0, all processors has completed operation.
			while(GlobalOperatingNumber)__nop();
			ExFreePool(IpbBuffer);
		}
	}
}