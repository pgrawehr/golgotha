// Filename: Processor.h
// =====================
// Author: Benjamin Jurke
// File history: 27.02.2002   File created.
///////////////////////////////////////////


#ifndef X86_PROCESSOR_H
#define X86_PROCESSOR_H


// Options:
///////////
#define PROCESSOR_FREQUENCY_MEASURE_AVAILABLE
// Includes <windows.h> --> code gets os-dependend (Win32)


typedef struct ProcessorExtensions
{
	bool FPU_FloatingPointUnit;
	bool VME_Virtual8086ModeEnhancements;
	bool DE_DebuggingExtensions;
	bool PSE_PageSizeExtensions;
	bool TSC_TimeStampCounter;
	bool MSR_ModelSpecificRegisters;
	bool PAE_PhysicalAddressExtension;
	bool MCE_MachineCheckException;
	bool CX8_COMPXCHG8B_Instruction;
	bool APIC_AdvancedProgrammableInterruptController;
	unsigned int APIC_ID;
	bool SEP_FastSystemCall;
	bool MTRR_MemoryTypeRangeRegisters;
	bool PGE_PTE_GlobalFlag;
	bool MCA_MachineCheckArchitecture;
	bool CMOV_ConditionalMoveAndCompareInstructions;
	bool FGPAT_PageAttributeTable;
	bool PSE36_36bitPageSizeExtension;
	bool PN_ProcessorSerialNumber;
	bool CLFSH_CFLUSH_Instruction;
	unsigned int CLFLUSH_InstructionCacheLineSize;
	bool DS_DebugStore;
	bool ACPI_ThermalMonitorAndClockControl;
	bool EMMX_MultimediaExtensions;
	bool MMX_MultimediaExtensions;
	bool FXSR_FastStreamingSIMD_ExtensionsSaveRestore;
	bool SSE_StreamingSIMD_Extensions;
	bool SSE2_StreamingSIMD2_Extensions;
	bool SS_SelfSnoop;
	bool HT_HyperThreading;
	unsigned int HT_HyterThreadingSiblings;
	bool TM_ThermalMonitor;
	bool IA64_Intel64BitArchitecture;
	bool _3DNOW_InstructionExtensions;
	bool _E3DNOW_InstructionExtensions;
	bool AA64_AMD64BitArchitecture;
} ProcessorExtensions;

typedef struct ProcessorCache
{
	bool bPresent;
	char strSize[32];
	unsigned int uiAssociativeWays;
	unsigned int uiLineSize;
	bool bSectored;
	char strCache[128];
} ProcessorCache;

typedef struct ProcessorL1Cache
{
    ProcessorCache Instruction;
	ProcessorCache Data;
} ProcessorL1Cache;

typedef struct ProcessorTLB
{
	bool bPresent;
	char strPageSize[32];
	unsigned int uiAssociativeWays;
	unsigned int uiEntries;
	char strTLB[128];
} ProcessorTLB;

typedef struct ProcessorInfo
{
	char strVendor[16];
	unsigned int uiFamily;
	unsigned int uiExtendedFamily;
	char strFamily[64];
	unsigned int uiModel;
	unsigned int uiExtendedModel;
	char strModel[128];
	unsigned int uiStepping;
	unsigned int uiType;
	char strType[64];
	unsigned int uiBrandID;
	char strBrandID[64];
	char strProcessorSerial[64];
	unsigned long MaxSupportedLevel;
	unsigned long MaxSupportedExtendedLevel;
	ProcessorExtensions _Ext;
	ProcessorL1Cache _L1;
	ProcessorCache _L2;
	ProcessorCache _L3;
	ProcessorCache _Trace;
	ProcessorTLB _Instruction;
	ProcessorTLB _Data;
} ProcessorInfo;


// CProcessor
// ==========
// Class for detecting the processor name, type and available
// extensions as long as it's speed.
/////////////////////////////////////////////////////////////
class CProcessor
{
// Constructor / Destructor:
////////////////////////////
public:
	CProcessor();

// Private vars:
////////////////
private:
	__int64 uqwFrequency;
	char strCPUName[128];
	ProcessorInfo CPUInfo;

// Private functions:
/////////////////////
private:
	bool AnalyzeIntelProcessor();
	bool AnalyzeAMDProcessor();
	bool AnalyzeUnknownProcessor();
	bool CheckCPUIDPresence();
	void DecodeProcessorConfiguration(unsigned int cfg);
	void TranslateProcessorConfiguration();
	void GetStandardProcessorConfiguration();
	void GetStandardProcessorExtensions();

// Public functions:
////////////////////
public:
	unsigned __int64 GetCPUFrequency(unsigned int uiMeasureMSecs);
	const ProcessorInfo *GetCPUInfo();
	bool CPUInfoToText(char *strBuffer, unsigned int uiMaxLen);
	bool WriteInfoTextFile(const char *strFilename);
};


#endif