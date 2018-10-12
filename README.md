# ipb
IPB - InterProcessor Broadcast
A simple broadcast generic call to all logical processors by DPC.

This project implement a method to broadcast an execution to all processors by "Defered Procedure Call" (a.k.a DPC) Mechanism.
We use a sequence of KeInitializeDpc, KeSetTargetProcessorDpc, KeSetImportanceDpc and KeInsertQueueDpc to insert DPCs to all logical processors.
All functions we used are exported since Windows Server 2000, so the compatibility of this project is considerably good.
