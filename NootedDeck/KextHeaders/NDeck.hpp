// NootedDeck
// 🛈 brurmonemt 2025
//  
// File: NootedDeck/KextHeaders/NDeck.hpp
// main class header (NootedDeck/NDeck.cpp)

#pragma once
#include <Headers/kern_patcher.hpp>
#include <IOKit/pci/IOPCIDevice.h>
#include <KextHeaders/NDeckAttributes.hpp>

class NDeck
{
    bool ModuleLoaded          {false};
    NDeckAttributes Attributes {};
    IOPCIDevice *GPU           {nullptr};
    IOMemoryMap *rMMIO         {nullptr};
    volatile UInt32 *rMMIOPtr  {nullptr};
    UInt32 DeviceID            {0};
    UInt32 PCIRev              {0};
    UInt32 FBOffset            {0};
    UInt32 DeviceRev           {0};
    UInt32 EnumRev             {0};
    
    mach_vm_address_t OrgAddDrivers   {0};    // might have to be moved to separate modules!
    mach_vm_address_t OrgSafeMetaCast {0};

    public:
        static NDeck &Singleton();
        void StartModule();
        void InitHWLate();

        void ProcessPatcher(KernelPatcher &Patcher);

        void SetProperty32Bit(const char *Key, UInt32 Value);
        UInt32 ReadRegion32Bit(UInt32 Region) const;
        void WriteReg32Bit(UInt32 Region, UInt32 Value) const;
        UInt32 SMUWaitForResponse() const;
        CAILResult SendMsgToSMC(UInt32 Message, UInt32 Parameter = 0, UInt32 *OutParameter = nullptr) const;

        template<typename T>
        T *GetVBIOSDataTable(UInt32 Index) {
            auto *VBIOS = static_cast<const UInt8 *>(this->VBIOSData->getBytesNoCopy());
            auto Base = *reinterpret_cast<const UInt16 *>(VBIOS + ATOM_ROM_TABLE_PTR);
            auto DataTable = *reinterpret_cast<const UInt16 *>(VBIOS + Base + ATOM_ROM_DATA_PTR);
            auto *MDT = reinterpret_cast<const UInt16 *>(VBIOS + DataTable + 4);
            auto Offset = MDT[Index];
            return Offset ? reinterpret_cast<T *>(const_cast<UInt8 *>(VBIOS) + Offset) : nullptr;
        }

        // temp hack... i think
        OSMetaClass *MetaClassMap[5][2] = {{nullptr}};

    private:
        bool GetVBIOSFromVFCT(bool Strict);
        bool GetVBIOSFromVRAM();
        bool GetVBIOSFromExpansionROM();
        bool GetVBIOS();

        static bool WrapAddDrivers(void *ThatIGuess, OSArray *Array, bool DoNubMatching);
        static OSMetaClassBase *WrapSafeMetaCast(const OSMetaClassBase *AnObject, const OSMetaClass *ToMeta);
};
