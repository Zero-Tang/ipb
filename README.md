# ipb
IPB - InterProcessor Broadcast
A simple broadcast generic call to all logical processors by DPC.

This project implement a method to broadcast an execution to all processors by "Defered Procedure Call" (a.k.a DPC) Mechanism.
We use a sequence of KeInitializeDpc, KeSetTargetProcessorDpc, KeSetImportanceDpc and KeInsertQueueDpc to insert DPCs to all logical processors.
All functions we used are exported since Windows Server 2000, so the compatibility of this project is considerably good.

To build this project, you may use Visual Studio 2010 and WDK7600. (Note that you should install WDK7600 to default path on C disk).
I have already setup the compiling parameters, so you may simply click on "Build Solution" to build it.
