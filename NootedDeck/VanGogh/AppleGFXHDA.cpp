// TODO: label this file

#include <Headers/kern_api.hpp>
#include <KextHeaders/NDeck.hpp>
#include <KextHeaders/Patcher+.hpp>
#include <KextHeaders/VanGogh/AppleGFXHDA.hpp>

constexpr UInt32 AMDVendorID = 0x1002;
constexpr UInt32 Navi10HDMIDeviceID = 0xAB38; // This is a placeholder (shocker)
constexpr UInt32 Navi10HDMIID = (Navi10HDMIDeviceID << 16) | AMDVendorID;
constexpr UInt32 AerithHDMIDeviceID = 0x163F;
constexpr UInt32 AerithHDMIID = (AerithHDMIDeviceID << 16) | AMDVendorID;
constexpr UInt32 SephirothHDMIDeviceID = 0x1640;
constexpr UInt32 SephirothHDMIID = (SephirothHDMIDeviceID << 16) | AMDVendorID;

static const char *AppleGFXHDAPath = "/System/Library/Extensions/AppleGFXHDA.kext/Contents/MacOS/AppleGFXHDA";

static KernelPatcher::KextInfo AppleGFXHDAKext
{
    "com.apple.driver.AppleGFXHDA",
    &AppleGFXHDAPath,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};

// Module logic

static VanGogh::AppleGFXHDA Instance {};

VanGogh::AppleGFXHDA &VanGogh::AppleGFXHDA::Singleton() { return Instance; }

void VanGogh::AppleGFXHDA::StartModule()
{
    PANIC_COND(this->ModuleLoaded, "AGFXHDA", "Attempted to load module twice!");
    this->ModuleLoaded = true;

    SYSLOG("AGFXHDA", "Module loaded successfully!")

    lilu.onKextLoadForce(
        &AppleGFXHDAKext, 1,
        [](void *User, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
        {
            static_cast<VanGogh::AppleGFXHDA *>(User)->ProcessKext(Patcher, ID, Slide, Size);
        },
    this);
}

void VanGogh::AppleGFXHDA::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    if (AppleGFXHDAKext.loadIndex != ID) { return; }

    NDeck::Singleton().InitHWLate();

    const UInt32 ProbeFind = Navi10HDMIID;
    const UInt32 ProbeRepl = NDeck::Singleton().GetAttributes().IsAerith() ? AerithHDMIID : SephirothHDMIID;
    const KernelPatcher::LookupPatch Patch {&AppleGFXHDAKext, reinterpret_cast<const UInt8 *>(&ProbeFind),
        reinterpret_cast<const UInt8 *>(&ProbeRepl), sizeof(ProbeFind), 1};
    Patcher.clearError();
    Patcher.applyLookupPatch(&Patch);
    PANIC_COND(Patcher.getError() != KernelPatcher::Error::NoError, "AGFXHDA",
        "Failed to apply patch for HDMI controller probe");

    KernelPatcher::SolveRequest SolveRequests[] =
    {
        {"__ZN34AppleGFXHDAFunctionGroupATI_Tahiti10gMetaClassE", this->OrgFunctionGroupTahiti},
        {"__ZN26AppleGFXHDAWidget_1002AAA010gMetaClassE", this->OrgWidget1002AAA0},
    };
    PANIC_COND(!Patcher.solveMultiple(ID, SolveRequests, Slide, Size), "AGFXHDA", "Failed to solve symbols");

    KernelPatcher::RouteRequest Requests[] =
    {
        {"__ZN31AppleGFXHDAFunctionGroupFactory27createAppleHDAFunctionGroupEP11DevIdStruct",
            WrapCreateAppleHDAFunctionGroup, this->OrgCreateAppleHDAFunctionGroup},
        {"__ZN24AppleGFXHDAWidgetFactory20createAppleHDAWidgetEP11DevIdStruct", WrapCreateAppleHDAWidget,
            this->OrgCreateAppleHDAWidget},
    };
    PANIC_COND(!Patcher.routeMultiple(ID, Requests, Slide, Size), "AGFXHDA", "Failed to route symbols");
}

void *VanGogh::AppleGFXHDA::WrapCreateAppleHDAFunctionGroup(void *DevID)
{
    auto VendorID = getMember<UInt16>(DevID, 0x2);
    auto DeviceID = getMember<UInt32>(DevID, 0x8);
    if (VendorID == AMDVendorID && (DeviceID == AerithHDMIDeviceID || DeviceID == SephirothHDMIDeviceID))
    {
        return Singleton().OrgFunctionGroupTahiti->alloc();
    }
    return FunctionCast(WrapCreateAppleHDAFunctionGroup, Singleton().OrgCreateAppleHDAFunctionGroup)(DevID);
}

void *VanGogh::AppleGFXHDA::WrapCreateAppleHDAWidget(void *DevID)
{
    auto VendorID = getMember<UInt16>(DevID, 0x2);
    auto DeviceID = getMember<UInt32>(DevID, 0x8);
    if (VendorID == AMDVendorID && (DeviceID == AerithHDMIDeviceID || DeviceID == SephirothHDMIDeviceID))
    {
        return Singleton().OrgWidget1002AAA0->alloc();
    }
    return FunctionCast(WrapCreateAppleHDAFunctionGroup, Singleton().OrgCreateAppleHDAFunctionGroup)(DevID);
}