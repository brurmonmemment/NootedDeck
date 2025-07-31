// NootedDeck
// 🛈 brurmonemt 2025
// © ChefKiss 2022-25
// 
// File: NootedDeck/NDeck.cpp
// main class

#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>
#include <Headers/kern_iokit.hpp>
#include <IOKit/acpi/IOACPIPlatformExpert.h>
#include <KextHeaders/Backlight.hpp>
#include <KextHeaders/DebugEnabler.hpp>
#include <KextHeaders/Hotfixes/AGDP.hpp>
#include <KextHeaders/Hotfixes/X6000FB.hpp>
#include <KextHeaders/SKU.hpp>
#include <KextHeaders/NDeck.hpp>
#include <KextHeaders/Patcher+.hpp>

// Module logic

static NDeck Instance {};

NDeck &NDeck::Singleton() { return Instance; }

void NDeck::StartModule()
{
    PANIC_COND(this->ModuleLoaded, "NDeck", "Attempted to load module twice!");
    this->ModuleLoaded = true;

    SYSLOG("NDeck", "🛈 brurmonemt 2025. NootedDeck is based on NootedRed/NootRX. No infringement intended.");
    SYSLOG("NDeck", "If you've paid for this, NootedRed or NootRX, try to ask fpr your money back. You've been scammed.");

    switch (getKernelVersion())
    {
        /* case KernelVersion::Catalina:    -- APU logic implemented in Big Sur above allegedly, need to check myself
            this->Attributes.SetCatalina(); -- but for now, commented out
            break; */
        case KernelVersion::BigSur:
            this->Attributes.SetBigSurPlus();
            break;
        case KernelVersion::Monterey:
            this->Attributes.SetBigSurPlus();
            this->Attributes.SetMonterey();
            this->Attributes.SetMontereyPlus();
            break;
        case KernelVersion::Ventura:
            this->Attributes.SetBigSurPlus();
            this->Attributes.SetMontereyPlus();
            this->Attributes.SetVentura();
            this->Attributes.SetVenturaPlus();
            if (getKernelMinorVersion() >= 5)
            {
                this->Attributes.SetVentura1304Based();
                this->Attributes.SetVentura1304Plus();
            }
            break;
        case KernelVersion::Sonoma:
            this->Attributes.SetBigSurPlus();
            this->Attributes.SetMontereyPlus();
            this->Attributes.SetVenturaPlus();
            this->Attributes.SetVentura1304Plus();
            if (getKernelMinorVersion() >= 4) { this->Attributes.SetSonoma1404Plus(); }
            break;
        case KernelVersion::Sequoia:
        case KernelVersion::Tahoe:
            this->Attributes.SetBigSurPlus();
            this->Attributes.SetMontereyPlus();
            this->Attributes.SetVenturaPlus();
            this->Attributes.SetVentura1304Plus();
            this->Attributes.SetSonoma1404Plus();
            break;
        default:
            PANIC("NDeck", "Unknown kernel version %d!", getKernelVersion());
    }
    
    SYSLOG("NDeck", "Module loaded successfully!")
    // DBGLOG("NDeck", "Catalina = %s", this->Attributes.IsCatalina() ? "Yes" : "No");    like i said before, no catalina yet
    DBGLOG("NDeck", "BigSurPlus = %s", this->Attributes.IsBigSurPlus() ? "Yes" : "No");
    DBGLOG("NDeck", "Monterey = %s", this->Attributes.IsMonterey() ? "Yes" : "No");
    DBGLOG("NDeck", "MontereyPlus = %s", this->Attributes.IsMontereyPlus() ? "Yes" : "No");
    DBGLOG("NDeck", "Ventura = %s", this->Attributes.IsVentura() ? "Yes" : "No");
    DBGLOG("NDeck", "VenturaPlus = %s", this->Attributes.IsVenturaPlus() ? "Yes" : "No");
    DBGLOG("NDeck", "Ventura1304Based = %s", this->Attributes.IsVentura1304Based() ? "Yes" : "No");
    DBGLOG("NDeck", "Ventura1304Plus = %s", this->Attributes.IsVentura1304Plus() ? "Yes" : "No");
    DBGLOG("NDeck", "Sonoma1404Plus = %s", this->Attributes.IsSonoma1404Plus() ? "Yes" : "No");
    DBGLOG("NDeck", "If any of the values above look incorrect, please mention this on GitHub via Issues.");

    Hotfixes::AGDP::Singleton().StartModule();
    Hotfixes::X6000FB::Singleton().StartModule();
    Backlight::Singleton().StartModule();
    DebugEnabler::Singleton().StartModule();

    lilu.onPatcherLoadForce([](void *User, KernelPatcher &Patcher) { static_cast<NDeck *>(User)->ProcessPatcher(Patcher); }, this);
}

void NDeck::InitHWLate()
{
    if (this->rMMIO != nullptr) { return; }

    this->GPU->setMemoryEnable(true);
    this->GPU->setBusMasterEnable(true);

    this->rMMIO = this->GPU->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress5,
                                                         kIOMapInhibitCache | kIOMapAnywhere);
                                                         
    this->rMMIOPtr = reinterpret_cast<UInt32 *>(this->rMMIO->getVirtualAddress());

    this->FBOffset  = static_cast<UInt64>(this->ReadRegion32Bit(GC_BASE_0 + MC_VM_FB_OFFSET)) << 24;
    this->DeviceRev = (this->ReadRegion32Bit(NBIO_BASE_2 + RCC_DEV0_EPF0_STRAP0) & RCC_DEV0_EPF0_STRAP0_ATI_REV_ID_MASK) >>
                       RCC_DEV0_EPF0_STRAP0_ATI_REV_ID_SHIFT;

    // this of course needs to be reworked lol
    /* if (this->DeviceRev == 0) 
    {
        this->variant = VGRev::Aerith;
    }
    else if (this->DeviceRev == 1)
    {
        this->variant = VGRev::Sephiroth;
    }
    else
    {
        this->variant = VGRev::Unknown;
    } */
}

static void UpdateProperties(IOPCIDevice *PCIDevice)
{
    UInt8 BuiltIn[] = {0x00};
    PCIDevice->setProperty("built-in", BuiltIn, arrsize(BuiltIn));
    auto VendorID = WIOKit::readPCIConfigValue(PCIDevice, WIOKit::kIOPCIConfigVendorID);
    auto DeviceID = WIOKit::readPCIConfigValue(PCIDevice, WIOKit::kIOPCIConfigDeviceID);

    auto *SKU = GetSKUForDevice(PCIDevice);
    if (SKU == nullptr)
    {
        SYSLOG("NDeck", "[%X:%X] WARNING: Unable to get device SKU", VendorID, DeviceID);
        return;
    }

    auto SKULength = static_cast<UInt32>(strLength(SKU) + 1);
    PCIDevice->setProperty("model", const_cast<char *>(SKU), SKULength);

    PCIDevice->setProperty("ATY,FamilyName", const_cast<char *>("Van Gogh"), 9);
    PCIDevice->setProperty("ATY,DeviceName", const_cast<char *>(SKU) + 13, SKULength - 13);
    PCIDevice->setProperty("AAPL,slot-name", const_cast<char *>("built-in"), 9);
}

void NDeck::ProcessPatcher(KernelPatcher &Patcher)
{
    auto *DevInfo = DeviceInfo::create();
    PANIC_COND(DevInfo == nullptr, "NDeck", "Failed to create device info!");
    DevInfo->processSwitchOff();

    this->GPU = OSDynamicCast(IOPCIDevice, DevInfo->VideoBuiltIn);
    PANIC_COND(this->GPU == nullptr, "NDeck", "VideoBuiltIn is not an IOPCIDevice!");
    PANIC_COND(WIOKit::readPCIConfigValue(this->GPU, WIOKit::kIOPCIConfigVendorID) != WIOKit::VendorID::ATIAMD, "NDeck",
        "VideoBuiltIn is not an AMD device!");

    WIOKit::renameDevice(this->GPU, "IGPU");
    WIOKit::awaitPublishing(this->GPU);
    UpdateProperties(this->GPU);
    
    this->DeviceID = WIOKit::readPCIConfigValue(this->GPU, WIOKit::kIOPCIConfigDeviceID);
    switch (this->DeviceID)
    {
        case 0x163F:
            this->Attributes.Variant = "Aerith";
            break;
        case 0x1640:
            this->Attributes.Variant = "Sephiroth";
            break;
        default:
            PANIC("NDeck", "Unknown device ID: 0x%X", this->DeviceID);
    }
    this->PCIRev = WIOKit::readPCIConfigValue(this->GPU, WIOKit::kIOPCIConfigRevisionID);

    char Name[128];
    bzero(Name, sizeof(Name));
    for (size_t i = 0, j = 0; i < DevInfo->videoExternal.size(); i++)
    {
        auto *Device = OSDynamicCast(IOPCIDevice, DevInfo->videoExternal[i].video);
        if (Device == nullptr) { continue; }

        snprintf(Name, arrsize(Name), "GFX%zu", j++);
        WIOKit::renameDevice(Device, Name);
        WIOKit::awaitPublishing(Device);
        updatePropertiesForDevice(Device);
    }

    DeviceInfo::deleter(DevInfo);

    KernelPatcher::RouteRequest Requests[] = 
    {
        {"__ZN15OSMetaClassBase12safeMetaCastEPKS_PK11OSMetaClass", WrapSafeMetaCast, this->OrgSafeMetaCast},
        {"__ZN11IOCatalogue10addDriversEP7OSArrayb", WrapAddDrivers, this->OrgAddDrivers},
    };

    PANIC_COND(!Patcher.routeMultipleLong(KernelPatcher::KernelID, Requests), "NDeck", "Failed to route kernel symbols");
}

void NDeck::SetProperty32Bit(const char *Key, UInt32 Value) { this->GPU->setProperty(Key, Value, 32); }

UInt32 NDeck::ReadRegion32Bit(UInt32 Region) const
{
    if ((Region * sizeof(UInt32)) < this->rMMIO->getLength())
    {
        return this->rMMIOPtr[Region];
    }
    else 
    {
        this->rMMIOPtr[PCIE_INDEX2] = Region;
        return this->rMMIOPtr[PCIE_DATA2];
    }
}

void NDeck::WriteRegion32Bit(UInt32 Region, UInt32 Value) const
{
    if ((Region * sizeof(UInt32)) < this->rMMIO->getLength())
    {
        this->rMMIOPtr[Region] = Value;
    }
    else
    {
        this->rMMIOPtr[PCIE_INDEX2] = Region;
        this->rMMIOPtr[PCIE_DATA2] = Value;
    }
}

UInt32 NDeck::SMUWaitForResponse() const
{
    UInt32 ReturnValue = AMDSMUFWResponse::KSMUFWResponseNoResponse;
    for (UInt32 i = 0; i < AMD_MAX_USEC_TIMEOUT; i++)
    {
        ReturnValue = this->ReadRegion32Bit(MP0_BASE_0 + MP1_SMN_C2PMSG_90);
        if (ReturnValue != AMDSMUFWResponse::KSMUFWResponseNoResponse) { break; }

        IOSleep(1);
    }

    return ReturnValue;
}

CAILResult NDeck::SendMessageToSMC(UInt32 Message, UInt32 Parameter, UInt32 *OutParameter) const {
    this->SMUWaitForResponse();

    this->WriteRegion32Bit(MP0_BASE_0 + MP1_SMN_C2PMSG_82, Parameter);
    this->WriteRegion32Bit(MP0_BASE_0 + MP1_SMN_C2PMSG_90, 0);
    this->WriteRegion32Bit(MP0_BASE_0 + MP1_SMN_C2PMSG_66, Message);

    const auto Response = this->SMUWaitForResponse();

    if (OutParameter != nullptr) { *OutParameter = this->ReadRegion32Bit(MP0_BASE_0 + MP1_SMN_C2PMSG_82); }

    return ProcessSMUFWResponse(Response);
}

static bool CheckATOMBIOS(const UInt8 *BIOS, size_t Size)
{
    if (Size < 0x49)
    {
        DBGLOG("NDeck", "VBIOS size is invalid!");
        return false;
    }

    if (BIOS[0] != 0x55 || BIOS[1] != 0xAA)
    {
        DBGLOG("NDeck", "VBIOS signature <%x %x> is invalid!", BIOS[0], BIOS[1]);
        return false;
    }

    UInt16 BIOSHeaderStart = BIOS[0x48] | static_cast<UInt16>(BIOS[0x49] << 8);
    if (!BIOSHeaderStart)
    {
        DBGLOG("NDeck", "Unable to locate VBIOS header!");
        return false;
    }

    UInt16 Temp = BIOSHeaderStart + 4;
    if (Size < Temp)
    {
        DBGLOG("NDeck", "BIOS header is uhh.. broken.");
        return false;
    }

    if (!memcmp(BIOS + Temp, "ATOM", 4) || !memcmp(BIOS + Temp, "MOTA", 4))
    {
        DBGLOG("NDeck", "ATOMBIOS detected!");
        return true;
    }

    return false;
}

// Twas a hack, says NootedRed!
class AppleACPIPlatformExpert : IOACPIPlatformExpert
{
    friend class NDeck;
};

bool NDeck::GetVBIOSFromVFCT(bool Strict)
{
    DBGLOG("NDeck", "Fetching VBIOS from VFCT table");
    auto *Expert = reinterpret_cast<AppleACPIPlatformExpert *>(this->GPU->getPlatform());
    PANIC_COND(Expert == nullptr, "NDeck", "Failed to get AppleACPIPlatformExpert");

    auto *VFCTData = Expert->getACPITableData("VFCT", 0);
    if (VFCTData == nullptr)
    {
        DBGLOG("NDeck", "No VFCT from AppleACPIPlatformExpert");
        return false;
    }

    auto *VFCT = static_cast<const VFCT *>(VFCTData->getBytesNoCopy());
    PANIC_COND(VFCT == nullptr, "NDeck", "VFCT OSData::getBytesNoCopy returned null");

    if (sizeof(VFCT) > VFCTData->getLength())
    {
        DBGLOG("NDeck", "VFCT table present but broken (too short).");
        return false;
    }

    auto Vendor         = WIOKit::readPCIConfigValue(this->GPU, WIOKit::kIOPCIConfigVendorID);
    auto BusNumber      = this->GPU->getBusNumber();
    auto DeviceNumber   = this->GPU->getDeviceNumber();
    auto DeviceFunction = this->GPU->getFunctionNumber();

    for (auto Offset = VFCT->vbiosImageOffset; Offset < VFCTData->getLength();)
    {
        auto *VHDR =
            static_cast<const GOPVideoBIOSHeader *>(VFCTData->getBytesNoCopy(Offset, sizeof(GOPVideoBIOSHeader)));
        if (VHDR == nullptr)
        {
            DBGLOG("NDeck", "VFCT header out of bounds");
            return false;
        }

        auto *VContent = static_cast<const UInt8 *>(
            VFCTData->getBytesNoCopy(Offset + sizeof(GOPVideoBIOSHeader), VHDR->imageLength));
        if (VContent == nullptr)
        {
            DBGLOG("NDeck", "VFCT VBIOS image out of bounds");
            return false;
        }

        Offset += sizeof(GOPVideoBIOSHeader) + VHDR->imageLength;

        if (VHDR->imageLength != 0 &&
            (!Strict || (VHDR->pciBus == BusNumber && VHDR->pciDevice == DeviceNumber && VHDR->pciFunction == DeviceFunction)) &&
            VHDR->vendorID == Vendor && VHDR->deviceID == this->deviceID)
        {
            if (CheckATOMBIOS(VContent, VHDR->imageLength))
            {
                this->VBIOSData = OSData::withBytes(VContent, VHDR->imageLength);
                PANIC_COND(this->VBIOSData == nullptr, "NDeck", "VFCT OSData::withBytes failed");
                return true;
            }

            DBGLOG("NDeck", "VFCT VBIOS is not an ATOMBIOS");
            return false;
        }
        else
        {
            DBGLOG("NDeck",
                "VFCT image does not match (pciBus: 0x%X pciDevice: 0x%X pciFunction: 0x%X "
                "VendorID: 0x%X deviceID: 0x%X) or length is 0 (imageLength: 0x%X)",
                VHDR->pciBus, VHDR->pciDevice, VHDR->pciFunction, VHDR->vendorID, VHDR->deviceID, VHDR->imageLength);
        }
    }

    DBGLOG("NDeck", "VFCT table present but broken");
    return false;
}

bool NDeck::GetVBIOSFromVRAM()
{
    auto *Bar0 =
        this->GPU->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0, kIOMapWriteCombineCache | kIOMapAnywhere);
    if (!Bar0 || !Bar0->getLength())
    {
        DBGLOG("NDeck", "FB BAR not enabled, unmapping Bar0...");
        OSSafeReleaseNULL(Bar0);
        return false;
    }

    auto *Framebuffer = reinterpret_cast<const UInt8 *>(Bar0->getVirtualAddress());
    UInt32 Size = 256 * 1024; // 256 KB (VBIOS ROM size)
    if (!CheckATOMBIOS(Framebuffer, Size))
    {
        DBGLOG("NDeck", "VRAM VBIOS is not an ATOMBIOS, unmapping Bar0...");
        OSSafeReleaseNULL(Bar0);
        return false;
    }

    this->VBIOSData = OSData::withBytes(Framebuffer, Size);
    PANIC_COND(this->VBIOSData == nullptr, "NDeck", "VRAM OSData::withBytes failed, unmapping Bar0...");
    OSSafeReleaseNULL(Bar0);
    return true;
}

bool NDeck::GetVBIOSFromExpansionROM()
{
    auto ExpansionROMBase = this->GPU->extendedConfigRead32(kIOPCIConfigExpansionROMBase);
    if (ExpansionROMBase == 0)
    {
        DBGLOG("NDeck", "No PCI Expansion ROM available");
        return false;
    }

    auto *ExpansionROM =
        this->GPU->mapDeviceMemoryWithRegister(kIOPCIConfigExpansionROMBase, kIOMapInhibitCache | kIOMapAnywhere);
    if (ExpansionROM == nullptr) { return false; }
    auto ExpansionROMLength = min(ExpansionROM->getLength(), ATOMBIOS_IMAGE_SIZE);
    if (ExpansionROMLength == 0)
    {
        DBGLOG("NDeck", "PCI Expansion ROM is empty");
        ExpansionROM->release();
        return false;
    }

    // Enable reading the expansion ROMs
    this->GPU->extendedConfigWrite32(kIOPCIConfigExpansionROMBase, ExpansionROMBase | 1);

    this->VBIOSData = OSData::withBytes(reinterpret_cast<const void *>(ExpansionROM->getVirtualAddress()),
        static_cast<UInt32>(ExpansionROMLength));
    PANIC_COND(this->VBIOSData == nullptr, "NDeck", "PCI Expansion ROM OSData::withBytes failed");
    ExpansionROM->release();

    // Disable reading the expansion ROMs
    this->GPU->extendedConfigWrite32(kIOPCIConfigExpansionROMBase, ExpansionROMBase);

    if (CheckATOMBIOS(static_cast<const UInt8 *>(this->VBIOSData->getBytesNoCopy()), ExpansionROMLength))
    {
        return true;
    }
    else
    {
        DBGLOG("NDeck", "PCI Expansion ROM VBIOS is not an ATOMBIOS");
        OSSafeReleaseNULL(this->VBIOSData);
        return false;
    }
}

bool NDeck::GetVBIOS()
{
    auto *BIOSImageProp = OSDynamicCast(OSData, this->GPU->getProperty("ATY,bin_image"));
    if (BIOSImageProp != nullptr)
    {
        if (CheckATOMBIOS(static_cast<const UInt8 *>(BIOSImageProp->getBytesNoCopy()), BIOSImageProp->getLength()))
        {
            this->VBIOSData = OSData::withData(BIOSImageProp);
            SYSLOG("NDeck", "Warning: VBIOS manually overridden, make sure you know what you're doing.");
            return true;
        }
        else
        {
            SYSLOG("NDeck", "Error: VBIOS override is invalid.");
        }
    }
    if (this->GetVBIOSFromVFCT(true))
    {
        DBGLOG("NDeck", "Got VBIOS from VFCT.");
    }
    else
    {
        SYSLOG("NDeck", "Failed to get VBIOS from VFCT, trying to get it from VRAM.");
        if (this->GetVBIOSFromVRAM())
        {
            DBGLOG("NDeck", "Got VBIOS from VRAM.");
        }
        else
        {
            SYSLOG("NDeck", "Failed to get VBIOS from VRAM, trying to get it from PCI Expansion ROM.");
            if (this->GetVBIOSFromExpansionROM())
            {
                DBGLOG("NDeck", "Got VBIOS from PCI Expansion ROM.");
            }
            else
            {
                SYSLOG("NDeck",
                    "Failed to get VBIOS from PCI Expansion ROM, trying to get it from VFCT (relaxed matches mode).");
                if (this->GetVBIOSFromVFCT(false))
                {
                    DBGLOG("NDeck", "Got VBIOS from VFCT (relaxed matches mode).");
                }
                else
                {
                    SYSLOG("NDeck", "Failed to get VBIOS from VFCT (relaxed matches mode).");
                    return false;
                }
            }
        }
    }
    return true;
}

static const char *GetDriverXMLForBundle(const char *BundleIdentifier, size_t *Length)
{
    const auto IdentifierLength = strLength(BundleIdentifier);
    const auto TotalLength = IdentifierLength + 5;
    auto *Filename = new char[TotalLength];
    memcpy(Filename, BundleIdentifier, IdentifierLength);
    strlcat(Filename, ".xml", TotalLength);

    const auto &DriversXML = getFirmwareNamed(Filename);
    delete[] Filename;

    *Length = DriversXML.Length;
    return reinterpret_cast<const char *>(DriversXML.data);
}

static const char *DriverBundleIdentifiers[] =
{
    "com.apple.kext.AMDRadeonX6000",
    "com.apple.kext.AMDRadeonX6000Framebuffer",
    "com.apple.driver.AppleGFXHDA",
};

static UInt8 MatchedDrivers = 0;

bool NDeck::WrapAddDrivers(void *ThatIGuess, OSArray *Array, bool DoNubMatching)
{
    UInt32 DriverCount = Array->getCount();
    for (UInt32 DriverIndex = 0; DriverIndex < DriverCount; DriverIndex += 1)
    {
        OSObject *Object = Array->getObject(DriverIndex);
        PANIC_COND(Object == nullptr, "NDeck", "Critical error in addDrivers: Index is out of bounds.");
        auto *Dictionary = OSDynamicCast(OSDictionary, Object);
        if (Dictionary == nullptr) { continue; }
        auto *BundleIdentifier = OSDynamicCast(OSString, Dictionary->getObject("CFBundleIdentifier"));
        if (BundleIdentifier == nullptr || BundleIdentifier->getLength() == 0) { continue; }
        auto *BundleIdentifierCStr = BundleIdentifier->getCStringNoCopy();
        if (BundleIdentifierCStr == nullptr) { continue; }

        for (size_t IdentifierIndex = 0; IdentifierIndex < arrsize(DriverBundleIdentifiers); IdentifierIndex += 1)
        {
            if ((MatchedDrivers & (1U << IdentifierIndex)) != 0) { continue; }

            if (strcmp(BundleIdentifierCStr, DriverBundleIdentifiers[IdentifierIndex]) == 0)
            {
                MatchedDrivers |= (1U << IdentifierIndex);

                DBGLOG("NDeck", "Matched %s, injecting.", BundleIdentifierCStr);

                size_t Length;
                auto *DriverXML = getDriverXMLForBundle(BundleIdentifierCStr, &Length);

                OSString *ErrorStr = nullptr;
                auto *DataUnserialized = OSUnserializeXML(DriverXML, Length, &ErrorStr);

                PANIC_COND(DataUnserialized == nullptr, "NDeck", "Failed to unserialize driver XML for %s: %s",
                    BundleIdentifierCStr, ErrorStr ? ErrorStr->getCStringNoCopy() : "(nil)");

                auto *Drivers = OSDynamicCast(OSArray, DataUnserialized);
                PANIC_COND(Drivers == nullptr, "NDeck", "Failed to cast %s driver data", BundleIdentifierCStr);
                UInt32 InjectedDriverCount = Drivers->getCount();

                Array->ensureCapacity(DriverCount + InjectedDriverCount);

                for (UInt32 injectedDriverIndex = 0; injectedDriverIndex < InjectedDriverCount;
                    injectedDriverIndex += 1)
                {
                    Array->setObject(DriverIndex, Drivers->getObject(injectedDriverIndex));
                    DriverIndex += 1;
                    DriverCount += 1;
                }

                DataUnserialized->release();
                break;
            }
        }
    }

    return FunctionCast(WrapAddDrivers, Singleton().OrgAddDrivers)(ThatIGuess, Array, DoNubMatching);
}

// i guess this has to be removed...
OSMetaClassBase *NDeck::WrapSafeMetaCast(const OSMetaClassBase *Object, const OSMetaClass *toMeta)
{
    auto ReturnValue = FunctionCast(WrapSafeMetaCast, Singleton().OrgSafeMetaCast)(Object, toMeta);

    if (LIKELY(ReturnValue)) { return ReturnValue; }

    for (const auto &ent : Singleton().metaClassMap)
    {
        if (UNLIKELY(ent[0] == toMeta))
        {
            return FunctionCast(WrapSafeMetaCast, Singleton().OrgSafeMetaCast)(Object, ent[1]);
        }
        else if (UNLIKELY(ent[1] == toMeta))
        {
            return FunctionCast(WrapSafeMetaCast, Singleton().OrgSafeMetaCast)(Object, ent[0]);
        }
    }

    return nullptr;
}