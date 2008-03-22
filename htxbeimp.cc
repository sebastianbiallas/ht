/*
 *	HT Editor
 *	htxbeimp.cc
 *
 *	Copyright (C) 2003 Stefan Esser
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "formats.h"
#include "htanaly.h"
#include "htctrl.h"
#include "data.h"
#include "endianess.h"
#include "htiobox.h"
#include "htpal.h"
#include "xbestruct.h"
#include "htxbe.h"
#include "htxbeimp.h"
#include "stream.h"
#include "strtools.h"
#include "httag.h"
#include "log.h"
#include "xbe_analy.h"
#include "snprintf.h"
#include "tools.h"

#include <stdlib.h>
#include <string.h>

static const char *xbox_exports[] = {
	NULL,
	"AvGetSavedDataAddress",                         //   1  80000001
	"AvSendTVEncoderOption",                         //   2  80000002
	"AvSetDisplayMode",                              //   3  80000003
	"AvSetSavedDataAddress",                         //   4  80000004
	"DbgBreakPoint",                                 //   5  80000005
	"DbgBreakPointWithStatus",                       //   6  80000006
	"DbgLoadImageSymbols",                           //   7  80000007
	"DbgPrint",                                      //   8  80000008
	"HalReadSMCTrayState",                           //   9  80000009
	"DbgPrompt",                                     //  10  8000000A
	"DbgUnLoadImageSymbols",                         //  11  8000000B
	"ExAcquireReadWriteLockExclusive",               //  12  8000000C
	"ExAcquireReadWriteLockShared",                  //  13  8000000D
	"ExAllocatePool",                                //  14  8000000E
	"ExAllocatePoolWithTag",                         //  15  8000000F
	"ExEventObjectType",                             //  16  80000010
	"ExFreePool",                                    //  17  80000011
	"ExInitializeReadWriteLock",                     //  18  80000012
	"ExInterlockedAddLargeInteger",                  //  19  80000013
	"ExInterlockedAddLargeStatistic",                //  20  80000014
	"ExInterlockedCompareExchange64",                //  21  80000015
	"ExMutantObjectType",                            //  22  80000016
	"ExQueryPoolBlockSize",                          //  23  80000017
	"ExQueryNonVolatileSetting",                     //  24  80000018
	"ExReadWriteRefurbInfo",                         //  25  80000019
	"ExRaiseException",                              //  26  8000001A
	"ExRaiseStatus",                                 //  27  8000001B
	"ExReleaseReadWriteLock",                        //  28  8000001C
	"ExSaveNonVolatileSetting",                      //  29  8000001D
	"ExSemaphoreObjectType",                         //  30  8000001E
	"ExTimerObjectType",                             //  31  8000001F
	"ExfInterlockedInsertHeadList",                  //  32  80000020
	"ExfInterlockedInsertTailList",                  //  33  80000021
	"ExfInterlockedRemoveHeadList",                  //  34  80000022
	"FscGetCacheSize",                               //  35  80000023
	"FscInvalidateIdleBlocks",                       //  36  80000024
	"FscSetCacheSize",                               //  37  80000025
	"HalClearSoftwareInterrupt",                     //  38  80000026
	"HalDisableSystemInterrupt",                     //  39  80000027
	"HalDiskCachePartitionCount",                    //  40  80000028
	"HalDiskModelNumber",                            //  41  80000029
	"HalDiskSerialNumber",                           //  42  8000002A
	"HalEnableSystemInterrupt",                      //  43  8000002B
	"HalGetInterruptVector",                         //  44  8000002C
	"HalReadSMBusValue",                             //  45  8000002D
	"HalReadWritePCISpace",                          //  46  8000002E
	"HalRegisterShutdownNotification",               //  47  8000002F
	"HalRequestSoftwareInterrupt",                   //  48  80000030
	"HalReturnToFirmware",                           //  49  80000031
	"HalWriteSMBusValue",                            //  50  80000032
	"InterlockedCompareExchange",                    //  51  80000033
	"InterlockedDecrement",                          //  52  80000034
	"InterlockedIncrement",                          //  53  80000035
	"InterlockedExchange",                           //  54  80000036
	"InterlockedExchangeAdd",                        //  55  80000037
	"InterlockedFlushSList",                         //  56  80000038
	"InterlockedPopEntrySList",                      //  57  80000039
	"InterlockedPushEntrySList",                     //  58  8000003A
	"IoAllocateIrp",                                 //  59  8000003B
	"IoBuildAsynchronousFsdRequest",                 //  60  8000003C
	"IoBuildDeviceIoControlRequest",                 //  61  8000003D
	"IoBuildSynchronousFsdRequest",                  //  62  8000003E
	"IoCheckShareAccess",                            //  63  8000003F
	"IoCompletionObjectType",                        //  64  80000040
	"IoCreateDevice",                                //  65  80000041
	"IoCreateFile",                                  //  66  80000042
	"IoCreateSymbolicLink",                          //  67  80000043
	"IoDeleteDevice",                                //  68  80000044
	"IoDeleteSymbolicLink",                          //  69  80000045
	"IoDeviceObjectType",                            //  70  80000046
	"IoFileObjectType",                              //  71  80000047
	"IoFreeIrp",                                     //  72  80000048
	"IoInitializeIrp",                               //  73  80000049
	"IoInvalidDeviceRequest",                        //  74  8000004A
	"IoQueryFileInformation",                        //  75  8000004B
	"IoQueryVolumeInformation",                      //  76  8000004C
	"IoQueueThreadIrp",                              //  77  8000004D
	"IoRemoveShareAccess",                           //  78  8000004E
	"IoSetIoCompletion",                             //  79  8000004F
	"IoSetShareAccess",                              //  80  80000050
	"IoStartNextPacket",                             //  81  80000051
	"IoStartNextPacketByKey",                        //  82  80000052
	"IoStartPacket",                                 //  83  80000053
	"IoSynchronousDeviceIoControlRequest",           //  84  80000054
	"IoSynchronousFsdRequest",                       //  85  80000055
	"IofCallDriver",                                 //  86  80000056
	"IofCompleteRequest",                            //  87  80000057
	"KdDebuggerEnabled",                             //  88  80000058
	"KdDebuggerNotPresent",                          //  89  80000059
	"IoDismountVolume",                              //  90  8000005A
	"IoDismountVolumeByName",                        //  91  8000005B
	"KeAlertResumeThread",                           //  92  8000005C
	"KeAlertThread",                                 //  93  8000005D
	"KeBoostPriorityThread",                         //  94  8000005E
	"KeBugCheck",                                    //  95  8000005F
	"KeBugCheckEx",                                  //  96  80000060
	"KeCancelTimer",                                 //  97  80000061
	"KeConnectInterrupt",                            //  98  80000062
	"KeDelayExecutionThread",                        //  99  80000063
	"KeDisconnectInterrupt",                         // 100  80000064
	"KeEnterCriticalRegion",                         // 101  80000065
	"MmGlobalData",                                  // 102  80000066
	"KeGetCurrentIrql",                              // 103  80000067
	"KeGetCurrentThread",                            // 104  80000068
	"KeInitializeApc",                               // 105  80000069
	"KeInitializeDeviceQueue",                       // 106  8000006A
	"KeInitializeDpc",                               // 107  8000006B
	"KeInitializeEvent",                             // 108  8000006C
	"KeInitializeInterrupt",                         // 109  8000006D
	"KeInitializeMutant",                            // 110  8000006E
	"KeInitializeQueue",                             // 111  8000006F
	"KeInitializeSemaphore",                         // 112  80000070
	"KeInitializeTimerEx",                           // 113  80000071
	"KeInsertByKeyDeviceQueue",                      // 114  80000072
	"KeInsertDeviceQueue",                           // 115  80000073
	"KeInsertHeadQueue",                             // 116  80000074
	"KeInsertQueue",                                 // 117  80000075
	"KeInsertQueueApc",                              // 118  80000076
	"KeInsertQueueDpc",                              // 119  80000077
	"KeInterruptTime",                               // 120  80000078
	"KeIsExecutingDpc",                              // 121  80000079
	"KeLeaveCriticalRegion",                         // 122  8000007A
	"KePulseEvent",                                  // 123  8000007B
	"KeQueryBasePriorityThread",                     // 124  8000007C
	"KeQueryInterruptTime",                          // 125  8000007D
	"KeQueryPerformanceCounter",                     // 126  8000007E
	"KeQueryPerformanceFrequency",                   // 127  8000007F
	"KeQuerySystemTime",                             // 128  80000080
	"KeRaiseIrqlToDpcLevel",                         // 129  80000081
	"KeRaiseIrqlToSynchLevel",                       // 130  80000082
	"KeReleaseMutant",                               // 131  80000083
	"KeReleaseSemaphore",                            // 132  80000084
	"KeRemoveByKeyDeviceQueue",                      // 133  80000085
	"KeRemoveDeviceQueue",                           // 134  80000086
	"KeRemoveEntryDeviceQueue",                      // 135  80000087
	"KeRemoveQueue",                                 // 136  80000088
	"KeRemoveQueueDpc",                              // 137  80000089
	"KeResetEvent",                                  // 138  8000008A
	"KeRestoreFloatingPointState",                   // 139  8000008B
	"KeResumeThread",                                // 140  8000008C
	"KeRundownQueue",                                // 141  8000008D
	"KeSaveFloatingPointState",                      // 142  8000008E
	"KeSetBasePriorityThread",                       // 143  8000008F
	"KeSetDisableBoostThread",                       // 144  80000090
	"KeSetEvent",                                    // 145  80000091
	"KeSetEventBoostPriority",                       // 146  80000092
	"KeSetPriorityProcess",                          // 147  80000093
	"KeSetPriorityThread",                           // 148  80000094
	"KeSetTimer",                                    // 149  80000095
	"KeSetTimerEx",                                  // 150  80000096
	"KeStallExecutionProcessor",                     // 151  80000097
	"KeSuspendThread",                               // 152  80000098
	"KeSynchronizeExecution",                        // 153  80000099
	"KeSystemTime",                                  // 154  8000009A
	"KeTestAlertThread",                             // 155  8000009B
	"KeTickCount",                                   // 156  8000009C
	"KeTimeIncrement",                               // 157  8000009D
	"KeWaitForMultipleObjects",                      // 158  8000009E
	"KeWaitForSingleObject",                         // 159  8000009F
	"KfRaiseIrql",                                   // 160  800000A0
	"KfLowerIrql",                                   // 161  800000A1
	"KiBugCheckData",                                // 162  800000A2
	"KiUnlockDispatcherDatabase",                    // 163  800000A3
	"LaunchDataPage",                                // 164  800000A4
	"MmAllocateContiguousMemory",                    // 165  800000A5
	"MmAllocateContiguousMemoryEx",                  // 166  800000A6
	"MmAllocateSystemMemory",                        // 167  800000A7
	"MmClaimGpuInstanceMemory",                      // 168  800000A8
	"MmCreateKernelStack",                           // 169  800000A9
	"MmDeleteKernelStack",                           // 170  800000AA
	"MmFreeContiguousMemory",                        // 171  800000AB
	"MmFreeSystemMemory",                            // 172  800000AC
	"MmGetPhysicalAddress",                          // 173  800000AD
	"MmIsAddressValid",                              // 174  800000AE
	"MmLockUnlockBufferPages",                       // 175  800000AF
	"MmLockUnlockPhysicalPage",                      // 176  800000B0
	"MmMapIoSpace",                                  // 177  800000B1
	"MmPersistContiguousMemory",                     // 178  800000B2
	"MmQueryAddressProtect",                         // 179  800000B3
	"MmQueryAllocationSize",                         // 180  800000B4
	"MmQueryStatistics",                             // 181  800000B5
	"MmSetAddressProtect",                           // 182  800000B6
	"MmUnmapIoSpace",                                // 183  800000B7
	"NtAllocateVirtualMemory",                       // 184  800000B8
	"NtCancelTimer",                                 // 185  800000B9
	"NtClearEvent",                                  // 186  800000BA
	"NtClose",                                       // 187  800000BB
	"NtCreateDirectoryObject",                       // 188  800000BC
	"NtCreateEvent",                                 // 189  800000BD
	"NtCreateFile",                                  // 190  800000BE
	"NtCreateIoCompletion",                          // 191  800000BF
	"NtCreateMutant",                                // 192  800000C0
	"NtCreateSemaphore",                             // 193  800000C1
	"NtCreateTimer",                                 // 194  800000C2
	"NtDeleteFile",                                  // 195  800000C3
	"NtDeviceIoControlFile",                         // 196  800000C4
	"NtDuplicateObject",                             // 197  800000C5
	"NtFlushBuffersFile",                            // 198  800000C6
	"NtFreeVirtualMemory",                           // 199  800000C7
	"NtFsControlFile",                               // 200  800000C8
	"NtOpenDirectoryObject",                         // 201  800000C9
	"NtOpenFile",                                    // 202  800000CA
	"NtOpenSymbolicLinkObject",                      // 203  800000CB
	"NtProtectVirtualMemory",                        // 204  800000CC
	"NtPulseEvent",                                  // 205  800000CD
	"NtQueueApcThread",                              // 206  800000CE
	"NtQueryDirectoryFile",                          // 207  800000CF
	"NtQueryDirectoryObject",                        // 208  800000D0
	"NtQueryEvent",                                  // 209  800000D1
	"NtQueryFullAttributesFile",                     // 210  800000D2
	"NtQueryInformationFile",                        // 211  800000D3
	"NtQueryIoCompletion",                           // 212  800000D4
	"NtQueryMutant",                                 // 213  800000D5
	"NtQuerySemaphore",                              // 214  800000D6
	"NtQuerySymbolicLinkObject",                     // 215  800000D7
	"NtQueryTimer",                                  // 216  800000D8
	"NtQueryVirtualMemory",                          // 217  800000D9
	"NtQueryVolumeInformationFile",                  // 218  800000DA
	"NtReadFile",                                    // 219  800000DB
	"NtReadFileScatter",                             // 220  800000DC
	"NtReleaseMutant",                               // 221  800000DD
	"NtReleaseSemaphore",                            // 222  800000DE
	"NtRemoveIoCompletion",                          // 223  800000DF
	"NtResumeThread",                                // 224  800000E0
	"NtSetEvent",                                    // 225  800000E1
	"NtSetInformationFile",                          // 226  800000E2
	"NtSetIoCompletion",                             // 227  800000E3
	"NtSetSystemTime",                               // 228  800000E4
	"NtSetTimerEx",                                  // 229  800000E5
	"NtSignalAndWaitForSingleObjectEx",              // 230  800000E6
	"NtSuspendThread",                               // 231  800000E7
	"NtUserIoApcDispatcher",                         // 232  800000E8
	"NtWaitForSingleObject",                         // 233  800000E9
	"NtWaitForSingleObjectEx",                       // 234  800000EA
	"NtWaitForMultipleObjectsEx",                    // 235  800000EB
	"NtWriteFile",                                   // 236  800000EC
	"NtWriteFileGather",                             // 237  800000ED
	"NtYieldExecution",                              // 238  800000EE
	"ObCreateObject",                                // 239  800000EF
	"ObDirectoryObjectType",                         // 240  800000F0
	"ObInsertObject",                                // 241  800000F1
	"ObMakeTemporaryObject",                         // 242  800000F2
	"ObOpenObjectByName",                            // 243  800000F3
	"ObOpenObjectByPointer",                         // 244  800000F4
	"ObpObjectHandleTable",                          // 245  800000F5
	"ObReferenceObjectByHandle",                     // 246  800000F6
	"ObReferenceObjectByName",                       // 247  800000F7
	"ObReferenceObjectByPointer",                    // 248  800000F8
	"ObSymbolicLinkObjectType",                      // 249  800000F9
	"ObfDereferenceObject",                          // 250  800000FA
	"ObfReferenceObject",                            // 251  800000FB
	"PhyGetLinkState",                               // 252  800000FC
	"PhyInitialize",                                 // 253  800000FD
	"PsCreateSystemThread",                          // 254  800000FE
	"PsCreateSystemThreadEx",                        // 255  800000FF
	"PsQueryStatistics",                             // 256  80000100
	"PsSetCreateThreadNotifyRoutine",                // 257  80000101
	"PsTerminateSystemThread",                       // 258  80000102
	"PsThreadObjectType",                            // 259  80000103
	"RtlAnsiStringToUnicodeString",                  // 260  80000104
	"RtlAppendStringToString",                       // 261  80000105
	"RtlAppendUnicodeStringToString",                // 262  80000106
	"RtlAppendUnicodeToString",                      // 263  80000107
	"RtlAssert",                                     // 264  80000108
	"RtlCaptureContext",                             // 265  80000109
	"RtlCaptureStackBackTrace",                      // 266  8000010A
	"RtlCharToInteger",                              // 267  8000010B
	"RtlCompareMemory",                              // 268  8000010C
	"RtlCompareMemoryUlong",                         // 269  8000010D
	"RtlCompareString",                              // 270  8000010E
	"RtlCompareUnicodeString",                       // 271  8000010F
	"RtlCopyString",                                 // 272  80000110
	"RtlCopyUnicodeString",                          // 273  80000111
	"RtlCreateUnicodeString",                        // 274  80000112
	"RtlDowncaseUnicodeChar",                        // 275  80000113
	"RtlDowncaseUnicodeString",                      // 276  80000114
	"RtlEnterCriticalSection",                       // 277  80000115
	"RtlEnterCriticalSectionAndRegion",              // 278  80000116
	"RtlEqualString",                                // 279  80000117
	"RtlEqualUnicodeString",                         // 280  80000118
	"RtlExtendedIntegerMultiply",                    // 281  80000119
	"RtlExtendedLargeIntegerDivide",                 // 282  8000011A
	"RtlExtendedMagicDivide",                        // 283  8000011B
	"RtlFillMemory",                                 // 284  8000011C
	"RtlFillMemoryUlong",                            // 285  8000011D
	"RtlFreeAnsiString",                             // 286  8000011E
	"RtlFreeUnicodeString",                          // 287  8000011F
	"RtlGetCallersAddress",                          // 288  80000120
	"RtlInitAnsiString",                             // 289  80000121
	"RtlInitUnicodeString",                          // 290  80000122
	"RtlInitializeCriticalSection",                  // 291  80000123
	"RtlIntegerToChar",                              // 292  80000124
	"RtlIntegerToUnicodeString",                     // 293  80000125
	"RtlLeaveCriticalSection",                       // 294  80000126
	"RtlLeaveCriticalSectionAndRegion",              // 295  80000127
	"RtlLowerChar",                                  // 296  80000128
	"RtlMapGenericMask",                             // 297  80000129
	"RtlMoveMemory",                                 // 298  8000012A
	"RtlMultiByteToUnicodeN",                        // 299  8000012B
	"RtlMultiByteToUnicodeSize",                     // 300  8000012C
	"RtlNtStatusToDosError",                         // 301  8000012D
	"RtlRaiseException",                             // 302  8000012E
	"RtlRaiseStatus",                                // 303  8000012F
	"RtlTimeFieldsToTime",                           // 304  80000130
	"RtlTimeToTimeFields",                           // 305  80000131
	"RtlTryEnterCriticalSection",                    // 306  80000132
	"RtlUlongByteSwap",                              // 307  80000133
	"RtlUnicodeStringToAnsiString",                  // 308  80000134
	"RtlUnicodeStringToInteger",                     // 309  80000135
	"RtlUnicodeToMultiByteN",                        // 310  80000136
	"RtlUnicodeToMultiByteSize",                     // 311  80000137
	"RtlUnwind",                                     // 312  80000138
	"RtlUpcaseUnicodeChar",                          // 313  80000139
	"RtlUpcaseUnicodeString",                        // 314  8000013A
	"RtlUpcaseUnicodeToMultiByteN",                  // 315  8000013B
	"RtlUpperChar",                                  // 316  8000013C
	"RtlUpperString",                                // 317  8000013D
	"RtlUshortByteSwap",                             // 318  8000013E
	"RtlWalkFrameChain",                             // 319  8000013F
	"RtlZeroMemory",                                 // 320  80000140
	"XboxEEPROMKey",                                 // 321  80000141
	"XboxHardwareInfo",                              // 322  80000142
	"XboxHDKey",                                     // 323  80000143
	"XboxKrnlVersion",                               // 324  80000144
	"XboxSignatureKey",                              // 325  80000145
	"XeImageFileName",                               // 326  80000146
	"XeLoadSection",                                 // 327  80000147
	"XeUnloadSection",                               // 328  80000148
	"READ_PORT_BUFFER_UCHAR",                        // 329  80000149
	"READ_PORT_BUFFER_USHORT",                       // 330  8000014A
	"READ_PORT_BUFFER_ULONG",                        // 331  8000014B
	"WRITE_PORT_BUFFER_UCHAR",                       // 332  8000014C
	"WRITE_PORT_BUFFER_USHORT",                      // 333  8000014D
	"WRITE_PORT_BUFFER_ULONG",                       // 334  8000014E
	"XcSHAInit",                                     // 335  8000014F
	"XcSHAUpdate",                                   // 336  80000150
	"XcSHAFinal",                                    // 337  80000151
	"XcRC4Key",                                      // 338  80000152
	"XcRC4Crypt",                                    // 339  80000153
	"XcHMAC",                                        // 340  80000154
	"XcPKEncPublic",                                 // 341  80000155
	"XcPKDecPrivate",                                // 342  80000156
	"XcPKGetKeyLen",                                 // 343  80000157
	"XcVerifyPKCS1Signature",                        // 344  80000158
	"XcModExp",                                      // 345  80000159
	"XcDESKeyParity",                                // 346  8000015A
	"XcKeyTable",                                    // 347  8000015B
	"XcBlockCrypt",                                  // 348  8000015C
	"XcBlockCryptCBC",                               // 349  8000015D
	"XcCryptService",                                // 350  8000015E
	"XcUpdateCrypto",                                // 351  8000015F
	"RtlRip",                                        // 352  80000160
	"XboxLANKey",                                    // 353  80000161
	"XboxAlternateSignatureKeys",                    // 354  80000162
	"XePublicKeyData",                               // 355  80000163
	"HalBootSMCVideoMode",                           // 356  80000164
	"IdexChannelObject",                             // 357  80000165
	"HalIsResetOrShutdownPending",                   // 358  80000166
	"IoMarkIrpMustComplete",                         // 359  80000167
	"HalInitiateShutdown",                           // 360  80000168
	"snprintf",                                      // 361  80000169
	"sprintf",                                       // 362  8000016A
	"vsnprintf",                                     // 363  8000016B
	"vsprintf",                                      // 364  8000016C
	"HalEnableSecureTrayEject",                      // 365  8000016D
	"HalWriteSMCScratchRegister"                     // 366  8000016E
};

static ht_view *htxbeimports_init(Bounds *b, File *file, ht_format_group *group)
{
	ht_xbe_shared_data *xbe_shared=(ht_xbe_shared_data *)group->get_shared_data();

	int h0=new_timer();
	start_timer(h0);

	ht_group *g;
	Bounds c;

	c=*b;
	g=new ht_group();
	g->init(&c, VO_RESIZE, DESC_XBE_IMPORTS"-g");
	ht_statictext *head;

	int function_count=0;

	c.y++;
	c.h--;
	ht_xbe_import_viewer *v=new ht_xbe_import_viewer();
	v->init(&c, DESC_XBE_IMPORTS, group);

	c.y--;
	c.h=1;

	FileOfs ofs;
	uint thunktablerva = xbe_shared->header.kernel_image_thunk_address - xbe_shared->header.base_address;
	uint *thunktable = ht_malloc(sizeof (xbox_exports));
	if (!thunktable) goto xbe_read_error;
	memset(thunktable, 0, sizeof(xbox_exports));

	if (!xbe_rva_to_ofs(&xbe_shared->sections, thunktablerva, &ofs))
		goto xbe_read_error;

	file->seek(ofs);
	if (file->read(thunktable, sizeof(xbox_exports)-4) != sizeof(xbox_exports)-4)
		goto xbe_read_error;

	for (; *thunktable; thunktable++, thunktablerva+=4) {
		uint ordinal;

		ordinal = createHostInt(thunktable, 4, little_endian);
		ht_xbe_import_function *func = new ht_xbe_import_function(thunktablerva, (char *)xbox_exports[ordinal & 0xfff], ordinal);
    		xbe_shared->imports.funcs->insert(func);
		function_count++;
	}
	

	stop_timer(h0);
//	LOG("%y: PE: %d ticks (%d msec) to read imports", file->get_name(), get_timer_tick(h0), get_timer_msec(h0));
	delete_timer(h0);

	char iline[256];
	ht_snprintf(iline, sizeof iline, "* XBE kernel thunk table at offset %08x (%d functions)", xbe_shared->header.kernel_image_thunk_address, function_count);
	head=new ht_statictext();
	head->init(&c, iline, align_left);

	g->insert(head);
	g->insert(v);
	//
	for (uint i=0; i<xbe_shared->imports.funcs->count(); i++) {
		ht_xbe_import_function *func = (ht_xbe_import_function*)(*xbe_shared->imports.funcs)[i];
		assert(func);
		char addr[32], name[256];
		ht_snprintf(addr, sizeof addr, "%08x", func->address);
		if (func->byname) {
			ht_snprintf(name, sizeof name, "%s", func->name.name);
		} else {
			ht_snprintf(name, sizeof name, "%04x (by ordinal)", func->ordinal);
		}
		v->insert_str(i, "NTOSKRNL.EXE", addr, name);
	}
	//
	v->update();

	g->setpalette(palkey_generic_window_default);

	xbe_shared->v_imports=v;
	return g;
xbe_read_error:
	delete_timer(h0);
	String fn;
	errorbox("%y: XBE import section seems to be corrupted.", &file->getFilename(fn));
	g->done();
	delete g;
	v->done();
	delete v;
	return NULL;
}

format_viewer_if htxbeimports_if = {
	htxbeimports_init,
	NULL
};

/*
 *	ht_xbe_import_function
 */
ht_xbe_import_function::ht_xbe_import_function(RVA a, uint o)
{
	ordinal = o;
	address = a;
	byname = false;
}

ht_xbe_import_function::ht_xbe_import_function(RVA a, char *n, uint h)
{
	name.name = ht_strdup(n);
	name.hint = h;
	address = a;
	byname = true;
}

ht_xbe_import_function::~ht_xbe_import_function()
{
	if (byname) free(name.name);
}

/*
 *	ht_xbe_import_viewer
 */
void ht_xbe_import_viewer::init(Bounds *b, const char *Desc, ht_format_group *fg)
{
	ht_text_listbox::init(b, 3, 2, LISTBOX_QUICKFIND);
	options |= VO_BROWSABLE;
	desc = strdup(Desc);
	format_group = fg;
	grouplib = false;
	sortby = 1;
	dosort();
}

void ht_xbe_import_viewer::done()
{
	ht_text_listbox::done();
}

void ht_xbe_import_viewer::dosort()
{
	ht_text_listbox_sort_order sortord[2];
	uint l, s;
	if (grouplib) {
		l = 0;
		s = 1;
	} else {
		l = 1;
		s = 0;
	}
	sortord[l].col = 0;
	sortord[l].compare_func = strcmp;
	sortord[s].col = sortby;
	sortord[s].compare_func = strcmp;
	sort(2, sortord);
}

const char *ht_xbe_import_viewer::func(uint i, bool execute)
{
	switch (i) {
		case 2:
			if (execute) {
				grouplib = !grouplib;
				dosort();
			}
			return grouplib ? (char*)"nbylib" : (char*)"bylib";
		case 4:
			if (execute) {
				if (sortby != 1) {
					sortby = 1;
					dosort();
				}
			}
			return "byaddr";
		case 5:
			if (execute) {
				if (sortby != 2) {
					sortby = 2;
					dosort();
				}
			}
			return "byname";
	}
	return NULL;
}

void ht_xbe_import_viewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_funcexec:
		if (func(msg->data1.integer, 1)) {
			clearmsg(msg);
			return;
		}
		break;
	case msg_funcquery: {
		const char *s=func(msg->data1.integer, 0);
		if (s) {
			msg->msg=msg_retval;
			msg->data1.cstr=s;
		}
		break;
	}
	case msg_keypressed: {
		if (msg->data1.integer == K_Return) {
			select_entry(e_cursor);
			clearmsg(msg);
		}
		break;
	}
	}
	ht_text_listbox::handlemsg(msg);
}

bool ht_xbe_import_viewer::select_entry(void *entry)
{
	ht_text_listbox_item *i = (ht_text_listbox_item *)entry;

	ht_xbe_shared_data *xbe_shared=(ht_xbe_shared_data *)format_group->get_shared_data();

	ht_xbe_import_function *e = (ht_xbe_import_function*)(*xbe_shared->imports.funcs)[i->id];
	if (!e) return true;
	if (xbe_shared->v_image) {
		ht_aviewer *av = (ht_aviewer*)xbe_shared->v_image;
		XBEAnalyser *a = (XBEAnalyser*)av->analy;
		Address *addr;
		addr = a->createAddress32(e->address+xbe_shared->header.base_address);
		if (av->gotoAddress(addr, NULL)) {
			app->focus(av);
			vstate_save();
		} else {
			global_analyser_address_string_format = ADDRESS_STRING_FORMAT_COMPACT | ADDRESS_STRING_FORMAT_ADD_0X;
			errorbox("can't follow: %s %y is not valid!", "import address", addr);
		}
		delete addr;
	} else errorbox("can't follow: no image viewer");
	return true;
}
