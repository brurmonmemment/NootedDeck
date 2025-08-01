// TODO: label this file

#pragma once
#include <Headers/kern_patcher.hpp>
#include <KextHeaders/AMDGPUDrivers/FB/AmdDeviceMemoryManager.hpp>

namespace VanGogh
{
    class X6000FB
    {
        using mapMemorySubRange_t = IOReturn (*)(void *ThatIGuess, AmdReservedMemorySelector Selector, size_t AtOffset,
            size_t WithSize, IOOptionBits AndAttributes);

        bool ModuleLoaded                           {false};
        bool FixedVBIOS                             {false};
        mach_vm_address_t OrgGetNumberOfConnectors  {0};
        mach_vm_address_t OrgIH40IVRingInitHardware {0};
        mach_vm_address_t OrgIRQMGRWriteRegister    {0};
        mach_vm_address_t OrgCreateRegisterAccess   {0};
        mapMemorySubRange_t OrgMapMemorySubRange    {nullptr};

        public:
            static X6000FB &Singleton();

            void StartModule();

        private:
            void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size);

            static UInt16 GetEnumeratedRevision();
            static IOReturn PopulateVRAMInfo(void *ThatIGuess, void *FwInfo);
            static UInt32 WrapGetNumberOfConnectors(void *ThatIGuess);
            static bool WrapIH40IVRingInitHardware(void *CTX, void *Param2);
            static void WrapIRQMGRWriteRegister(void *CTX, UInt64 Index, UInt32 Value);
            static void *WrapCreateRegisterAccess(void *InitData);
            static IOReturn IntializeReservedVRAM(void *ThatIGuess);    // AMD made this typo, not me
    };
};    // namespace VanGogh