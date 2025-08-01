// TODO: Label

#include <Headers/kern_api.hpp>
#include <KextHeaders/AMDGPUDrivers/VidMemType.hpp>
#include <KextHeaders/NDeck.hpp>
#include <KextHeaders/Patcher+.hpp>

static const char *RadeonX6000Framebuffer =
    "/System/Library/Extensions/AMDRadeonX6000Framebuffer.kext/Contents/MacOS/AMDRadeonX6000Framebuffer";

static KernelPatcher::KextInfo RadeonX6000FramebufferKext
{
    "com.apple.kext.AMDRadeonX6000Framebuffer",
    &RadeonX6000Framebuffer,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};

// Patterns

static const UInt8 KCailAsicCapsTablePattern[] = {0x6E, 0x00, 0x00, 0x00, 0x98, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

static const UInt8 KPopulateVramInfoPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x53, 0x48, 0x81, 0xEC,
    0x08, 0x01, 0x00, 0x00, 0x40, 0x89, 0xF0, 0x40, 0x89, 0xF0, 0x4C, 0x8D, 0xBD, 0xE0, 0xFE, 0xFF, 0xFF};
static const UInt8 KPopulateVramInfoPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static const UInt8 KGetNumberOfConnectorsPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x40, 0x8B, 0x40, 0x28, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x85, 0x00, 0x74, 0x00};
static const UInt8 KGetNumberOfConnectorsPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

static const UInt8 KIH40IVRingInitHardwarePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x40, 0x89, 0xF0, 0x49, 0x89, 0xF0, 0x40, 0x8B, 0x00, 0x00, 0x44, 0x00, 0x00};
static const UInt8 KIH40IVRingInitHardwarePatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xF0, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

static const UInt8 KIRQMGRWriteRegisterPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x41, 0x89, 0xD6, 0x49, 0x89, 0xF7, 0x48, 0x89, 0xFB, 0x48, 0x8B, 0x87, 0xB0, 0x00, 0x00, 0x00,
    0x48, 0x85, 0xC0};
static const UInt8 KIRQMGRWriteRegisterPattern1404[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x89, 0xD3, 0x49, 0x89, 0xF7, 0x49, 0x89, 0xFE, 0x48, 0x8B, 0x87, 0xB0, 0x00, 0x00, 0x00,
    0x48, 0x85, 0xC0};

// Fix register read (0xD31 -> 0xD2F) and family ID (0x8F -> 0x8E).
static const UInt8 KPopulateDeviceInfoOriginal[] {0xBE, 0x31, 0x0D, 0x00, 0x00, 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00,
    0xC7, 0x43, 0x00, 0x8F, 0x00, 0x00, 0x00};
static const UInt8 KPopulateDeviceInfoMask[] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 KPopulateDeviceInfoPatched[] {0xBE, 0x2F, 0x0D, 0x00, 0x00, 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00, 0xC7,
    0x43, 0x00, 0x8E, 0x00, 0x00, 0x00};

// Neutralize `AmdAtomVramInfo` creation null check.
// We don't have this entry in our VBIOS.
static const UInt8 KAMDAtomVramInfoNullCheckOriginal[] = {0x48, 0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0,
    0x0F, 0x84, 0x89, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7B, 0x18};
static const UInt8 KAMDAtomVramInfoNullCheckPatched[] = {0x48, 0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x90, 0x48, 0x8B, 0x7B, 0x18};

// Ditto
static const UInt8 KAMDAtomVramInfoNullCheckOriginal1015[] = {0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00, 0x48, 0x85,
    0xC0, 0x74, 0x00};
static const UInt8 KAMDAtomVramInfoNullCheckOriginalMask1015[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00};
static const UInt8 KAMDAtomVramInfoNullCheckPatched1015[] = {0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x90};

// Neutralize `AmdAtomPspDirectory` creation null check.
// We don't have this entry in our VBIOS.
static const UInt8 KAMDAtomPspDirectoryNullCheckOriginal[] = {0x48, 0x89, 0x83, 0x88, 0x00, 0x00, 0x00, 0x48, 0x85,
    0xC0, 0x0F, 0x84, 0xA1, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7B, 0x18};
static const UInt8 KAMDAtomPspDirectoryNullCheckPatched[] = {0x48, 0x89, 0x83, 0x88, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x90, 0x48, 0x8B, 0x7B, 0x18};

// Neutralize `AmdAtomVramInfo` null check.
static const UInt8 KGetFirmwareInfoNullCheckOriginal[] = {0x48, 0x83, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x84,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x89};
static const UInt8 KGetFirmwareInfoNullCheckOriginalMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static const UInt8 KGetFirmwareInfoNullCheckPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x90,
    0x66, 0x90, 0x66, 0x90, 0x00, 0x00};
static const UInt8 KGetFirmwareInfoNullCheckPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
static const UInt8 KGetFirmwareInfoNullCheckOriginal1404[] = {0x49, 0x83, 0xBC, 0x24, 0x90, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x49, 0x89};
static const UInt8 KGetFirmwareInfoNullCheckOriginalMask1404[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static const UInt8 KGetFirmwareInfoNullCheckPatched1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x00, 0x00};
static const UInt8 KGetFirmwareInfoNullCheckPatchedMask1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};

// Tell AGDC that we're an iGPU.
static const UInt8 KGetVendorInfoOriginal[]     = {0x48, 0x00, 0x02, 0x10, 0x00, 0x00, 0x02};
static const UInt8 KGetVendorInfoMask[]         = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 KGetVendorInfoPatched[]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const UInt8 KGetVendorInfoPatchedMask[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const UInt8 KGetVendorInfoOriginal1404[] = {0xC7, 0x00, 0x24, 0x02, 0x10, 0x00, 0x00, 0xC7, 0x00, 0x28, 0x02,
    0x00, 0x00, 0x00};
static const UInt8 KGetVendorInfoMask1404[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF};
static const UInt8 KGetVendorInfoPatched1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00};
static const UInt8 KGetVendorInfoPatchedMask1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00};

// Remove check for Navi family
static const UInt8 KInitializeDmcubServices1Original[] = {0x81, 0x79, 0x2C, 0x8F, 0x00, 0x00, 0x00};
static const UInt8 KInitializeDmcubServices1Patched[]  = {0x39, 0xC0, 0x66, 0x90, 0x66, 0x90, 0x90};

// Set DMCUB ASIC constant to DCN 2.1
static const UInt8 KInitializeDmcubServices2Original[] = {0x83, 0xC0, 0xC4, 0x83, 0xF8, 0x0A, 0xB8, 0x03, 0x00, 0x00,
    0x00, 0x83, 0xD0, 0x00};
static const UInt8 KInitializeDmcubServices2Patched[] = {0xB8, 0x02, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x90};

// Ditto, 14.4+
static const UInt8 KInitializeDmcubServices2Original1404[] = {0x83, 0xC0, 0xC4, 0x31, 0xC9, 0x83, 0xF8, 0x0A, 0x83,
    0xD1, 0x03};
static const UInt8 KInitializeDmcubServices2Patched1404[] = {0xB9, 0x02, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66, 0x90, 0x66,
    0x90};

/* // Ditto, 10.15
static const UInt8 KInitializeDmcubServices2Original1015[] = {0xC7, 0x46, 0x20, 0x01, 0x00, 0x00, 0x00};
static const UInt8 KInitializeDmcubServices2Patched1015[]  = {0xC7, 0x46, 0x20, 0x02, 0x00, 0x00, 0x00};

// 10.15: Set inst_const_size/bss_data_size to 0. To disable DMCUB firmware loading logic.
static const UInt8 KInitializeHardware1Original[] = {0x49, 0xBC, 0x00, 0x0A, 0x01, 0x00, 0xF4, 0x01, 0x00, 0x00};
static const UInt8 KInitializeHardware1Patched[]  = {0x49, 0xC7, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90};

// 10.15: Set fw_inst_const to nullptr, pt.2 of above.
static const UInt8 KInitializeHardware2Original[] = {0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x00, 0x10,
    0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C};
static const UInt8 KInitializeHardware2OriginalMask[] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const UInt8 KInitializeHardware2Patched[] = {0x49, 0xC7, 0xC5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UInt8 KInitializeHardware2PatchedMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// 10.15: Disable DMCUB firmware loading from DAL. HWLibs should be doing that.
static const UInt8 KAMDDalServicesInitializeOriginal[] = {0xBE, 0x01, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x00, 0x00, 0x60};
static const UInt8 KAMDDalServicesInitializeOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0x00, 0x00, 0xFF};
static const UInt8 KAMDDalServicesInitializePatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};
static const UInt8 KAMDDalServicesInitializePatchedMask[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00}; */ // Catalina Logic

/* // Raven: Change cursor and underflow tracker count to 4 instead of 6.
static const UInt8 KCreateControllerServicesOriginal[]     = {0x40, 0x00, 0x00, 0x40, 0x83, 0x00, 0x06};
static const UInt8 KCreateControllerServicesOriginalMask[] = {0xF0, 0x00, 0x00, 0xF0, 0xFF, 0x00, 0xFF};
static const UInt8 KCreateControllerServicesPatched[]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
static const UInt8 KCreateControllerServicesPatchedMask[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F};        // Commented out Raven logic for now, might or might not need later

// Ditto, 10.15.
static const UInt8 KCreateControllerServicesOriginal1015[]     = {0x48, 0x00, 0x00, 0x48, 0x83, 0x00, 0x05};
static const UInt8 KCreateControllerServicesOriginalMask1015[] = {0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF};
static const UInt8 KCreateControllerServicesPatched1015[]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
static const UInt8 KCreateControllerServicesPatchedMask1015[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F}; */ // More catalina shiz

/* // Raven: Change cursor count to 4 instead of 6.
static const UInt8 KSetupCursorsOriginal[]     = {0x40, 0x83, 0x00, 0x05};
static const UInt8 KSetupCursorsOriginalMask[] = {0xF0, 0xFF, 0x00, 0xFF};
static const UInt8 KSetupCursorsPatched[]      = {0x00, 0x00, 0x00, 0x03};
static const UInt8 KSetupCursorsPatchedMask[]  = {0x00, 0x00, 0x00, 0x0F}; */ // Commented out Raven logic for now, might or might not need later

// Ditto, 12.0+.
static const UInt8 KSetupCursorsOriginal12[]     = {0x40, 0x83, 0x00, 0x06};
static const UInt8 KSetupCursorsOriginalMask12[] = {0xF0, 0xFF, 0x00, 0xFF};
static const UInt8 KSetupCursorsPatched12[]      = {0x00, 0x00, 0x00, 0x04};
static const UInt8 KSetupCursorsPatchedMask12[]  = {0x00, 0x00, 0x00, 0x0F};

/* // Raven: Change link count to 4 instead of 6.
static const UInt8 KCreateLinksOriginal[]     = {0x06, 0x00, 0x00, 0x00, 0x40};
static const UInt8 KCreateLinksOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xF0};
static const UInt8 KCreateLinksPatched[]      = {0x04, 0x00, 0x00, 0x00, 0x00};
static const UInt8 KCreateLinksPatchedMask[]  = {0x0F, 0x00, 0x00, 0x00, 0x00}; */ // Commented out Raven logic for now, might or might not need later

// Module logic

static VanGogh::X6000FB Instance {};

VanGogh::X6000FB &VanGogh::X6000FB::Singleton() { return Instance; }

void VanGogh::X6000FB::StartModule()
{
    PANIC_COND(this->ModuleLoaded, "X6000FB", "Attempted to load module twice!");
    this->ModuleLoaded = true;

    SYSLOG("X6000FB", "Module loaded successfully!");

    lilu.onKextLoadForce(
        &RadeonX6000FramebufferKext, 1,
        [](void *User, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
        {
            static_cast<VanGogh::X6000FB *>(User)->ProcessKext(Patcher, ID, Slide, Size);
        },
    this);
}

void VanGogh::X6000FB::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    if (RadeonX6000FramebufferKext.loadIndex != ID) { return; }

    NDeck::Singleton().InitHWLate();

    CAILAsicCapsEntry *OrgAsicCapsTable = nullptr;
    Patcher+::PatternSolveRequest cailAsicCapsSolveRequest {"__ZL20CAIL_ASIC_CAPS_TABLE", OrgAsicCapsTable,
        KCailAsicCapsTablePattern};
    PANIC_COND(!cailAsicCapsSolveRequest.solve(Patcher, ID, Slide, Size), "X6000FB",
        "Failed to resolve CAIL_ASIC_CAPS_TABLE");

    Patcher+::PatternRouteRequest Requests[] =
    {
        {"__ZNK15AmdAtomVramInfo16populateVramInfoER16AtomFirmwareInfo", PopulateVramInfo, KPopulateVramInfoPattern,
            KPopulateVramInfoPatternMask},
        {"__ZNK32AMDRadeonX6000_AmdAsicInfoNavi1027getEnumeratedRevisionNumberEv", getEnumeratedRevision},
        {"__ZNK22AmdAtomObjectInfo_V1_421getNumberOfConnectorsEv", WrapGetNumberOfConnectors,
            this->orgGetNumberOfConnectors, KGetNumberOfConnectorsPattern, KGetNumberOfConnectorsPatternMask},
    };
    PANIC_COND(!Patcher+::PatternRouteRequest::RouteAll(Patcher, ID, Requests, Slide, Size), "X6000FB",
        "Failed to route symbols");

    if (NDeck::Singleton().GetAttributes().IsBigSurPlusB()) {
        KernelPatcher::RouteRequest Request {
            "__ZN32AMDRadeonX6000_AmdRegisterAccess20createRegisterAccessERNS_8InitDataE", WrapCreateRegisterAccess,
            this->orgCreateRegisterAccess};
        PANIC_COND(!Patcher.routeMultiple(ID, &Request, 1, Slide, Size), "X6000FB",
            "Failed to route createRegisterAccess");
    }

    const Patcher+::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, KPopulateDeviceInfoOriginal,
        KPopulateDeviceInfoMask, KPopulateDeviceInfoPatched, KPopulateDeviceInfoMask, 1};
    PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB", "Failed to apply populateDeviceInfo Patch");

    if (NDeck::Singleton().GetAttributes().IsSonoma1404PlusB())
    {
        const Patcher+::MaskedLookupPatch Patches[] =
        {
            {&RadeonX6000FramebufferKext, KGetFirmwareInfoNullCheckOriginal1404,
                KGetFirmwareInfoNullCheckOriginalMask1404, KGetFirmwareInfoNullCheckPatched1404,
                KGetFirmwareInfoNullCheckPatchedMask1404, 1},
            {&RadeonX6000FramebufferKext, KGetVendorInfoOriginal1404, KGetVendorInfoMask1404, KGetVendorInfoPatched1404,
                KGetVendorInfoPatchedMask1404, 1},
        };
        PANIC_COND(!Patcher+::MaskedLookupPatch::ApplyAll(Patcher, Patches, Slide, Size), "X6000FB",
            "Failed to apply patches (14.4)");
    }
    else
    {
        const Patcher+::MaskedLookupPatch Patches[] =
        {
            {&RadeonX6000FramebufferKext, KGetFirmwareInfoNullCheckOriginal, KGetFirmwareInfoNullCheckOriginalMask,
                KGetFirmwareInfoNullCheckPatched, KGetFirmwareInfoNullCheckPatchedMask, 1},
            {&RadeonX6000FramebufferKext, KGetVendorInfoOriginal, KGetVendorInfoMask, KGetVendorInfoPatched,
                KGetVendorInfoPatchedMask, 1},
        };
        PANIC_COND(!Patcher+::MaskedLookupPatch::ApplyAll(Patcher, Patches, Slide, Size), "X6000FB",
            "Failed to apply patches");
    }

    /* if (NDeck::Singleton().GetAttributes().IsCatalinaB())
    {
        const Patcher+::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, KAMDAtomVramInfoNullCheckOriginal1015,
            KAMDAtomVramInfoNullCheckOriginalMask1015, KAMDAtomVramInfoNullCheckPatched1015, 1};
        PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB", "Failed to apply null check Patch");
    } else {
        const Patcher+::MaskedLookupPatch Patches[] =
        {
            {&RadeonX6000FramebufferKext, KAMDAtomVramInfoNullCheckOriginal, KAMDAtomVramInfoNullCheckPatched, 1},
            {&RadeonX6000FramebufferKext, KAMDAtomPspDirectoryNullCheckOriginal, KAMDAtomPspDirectoryNullCheckPatched,
                1},
        };
        PANIC_COND(!Patcher+::MaskedLookupPatch::ApplyAll(Patcher, Patches, Slide, Size), "X6000FB",
            "Failed to apply null check patches");
    } */ // Might actually need later but remove the Catalina logic if it turns out Catalina wont cut

    PANIC_COND(MachInfo::setKernelWriting(true, KernelPatcher::kernelWriteLock) != KERN_SUCCESS, "X6000FB",
        "Failed to enable kernel writing");
    OrgAsicCapsTable->familyId = AMD_FAMILY_RAVEN;
    OrgAsicCapsTable->ddiCaps = NDeck::Singleton().GetAttributes().isRenoirE() ? ddiCapsRenoirE :
                                NDeck::Singleton().GetAttributes().isRenoir()  ? ddiCapsRenoir :
                                                                                ddiCapsRaven;
    OrgAsicCapsTable->deviceId = NDeck::Singleton().getDeviceID();
    OrgAsicCapsTable->revision = NDeck::Singleton().getDevRevision();
    OrgAsicCapsTable->extRevision =
        static_cast<UInt32>(NDeck::Singleton().getEnumRevision()) + NDeck::Singleton().getDevRevision();
    OrgAsicCapsTable->pciRevision = NDeck::Singleton().getPciRevision();
    MachInfo::setKernelWriting(false, KernelPatcher::kernelWriteLock);
    DBGLOG("X6000FB", "Applied DDI Caps patches");

    // XX: DCN 2 and newer have 6 display pipes, while DCN 1 (which is what Raven has) has only 4.
    // We need to patch the kext to create only 4 cursors, links and underflow trackers.
    
    // Actually no we do not because this is Van Gogh which uses DCN 3 Lolz!
}

UInt16 VanGogh::X6000FB::GetEnumeratedRevision() { return NDeck::Singleton().GetEnumRevision(); }

IOReturn VanGogh::X6000FB::PopulateVRAMInfo(void *, void *FwInfo) {
    UInt32 ChannelCount = 1;
    auto *Table = NDeck::Singleton().GetVBIOSDataTable<IGPSystemInfo>(0x1E);
    UInt8 MemoryType = 0;
    if (Table)
    {
        DBGLOG("X6000FB", "Fetching VRAM info from iGPU System Info");
        switch (Table->header.formatRev)
        {
            case 1:
                switch (Table->header.contentRev)
                {
                    case 11:
                    case 12:
                        if (Table->infoV11.umaChannelCount) { ChannelCount = Table->infoV11.umaChannelCount; }
                        MemoryType = Table->infoV11.MemoryType;
                        break;
                    default:
                        DBGLOG("X6000FB", "Unsupported contentRev %d", Table->header.contentRev);
                        break;
                }
                break;
            case 2:
                switch (Table->header.contentRev)
                {
                    case 1:
                    case 2:
                        if (Table->infoV2.umaChannelCount) { ChannelCount = Table->infoV2.umaChannelCount; }
                        MemoryType = Table->infoV2.MemoryType;
                        break;
                    default:
                        DBGLOG("X6000FB", "Unsupported contentRev %d", Table->header.contentRev);
                        break;
                }
                break;
            default:
                DBGLOG("X6000FB", "Unsupported formatRev %d", Table->header.formatRev);
                break;
        }
    }
    else
    {
        DBGLOG("X6000FB", "No iGPU System Info in Master Data Table");
    }
    auto &VideoMemoryType = getMember<UInt32>(FwInfo, 0x1C);
    switch (MemoryType)
    {
        case kLPDDR5MemType:
            // The boys over at AMD who made the macOS kexts doesn't know what DDR5 or LPDDR5 is so we'll default to DDR4
            VideoMemoryType = kVideoMemoryTypeDDR4;
            break;
        default:
            DBGLOG("X6000FB", "Unsupported memory type %d. Assuming DDR4", MemoryType);
            VideoMemoryType = kVideoMemoryTypeDDR4;
            break;
    }
    getMember<UInt32>(FwInfo, 0x20) = ChannelCount * 64;    // VRAM Width (64-bit channels)
    return kIOReturnSuccess;
}

UInt32 VanGogh::X6000FB::WrapGetNumberOfConnectors(void *ThatIGuess)
{
    if (!Singleton().fixedVBIOS)
    {
        Singleton().fixedVBIOS = true;
        struct DispObjInfoTableV1_4 *objInfo = getMember<DispObjInfoTableV1_4 *>(ThatIGuess, 0x28);
        if (objInfo->formatRev == 1 && (objInfo->contentRev == 4 || objInfo->contentRev == 5))
        {
            DBGLOG("X6000FB", "getNumberOfConnectors: Fixing VBIOS connectors");
            auto n = objInfo->pathCount;
            for (size_t i = 0, j = 0; i < n; i++)
            {
                // Skip invalid device tags
                if (objInfo->paths[i].devTag) {
                    objInfo->paths[j++] = objInfo->paths[i];
                } else {
                    objInfo->pathCount--;
                }
            }
        }
    }
    return FunctionCast(WrapGetNumberOfConnectors, Singleton().orgGetNumberOfConnectors)(ThatIGuess);
}

bool VanGogh::X6000FB::WrapIH40IVRingInitHardware(void *CTX, void *Param2)
{
    auto ReturnValue = FunctionCast(WrapIH40IVRingInitHardware, Singleton().orgIH40IVRingInitHardware)(CTX, Param2);
    NDeck::Singleton().WriteRegion32Bit(IH_CHICKEN, NDeck::Singleton().ReadRegion32Bit(IH_CHICKEN) | IH_MC_SPACE_GPA_ENABLE);
    return ReturnValue;
}

void VanGogh::X6000FB::WrapIRQMGRWriteRegister(void *CTX, UInt64 Index, UInt32 Value)
{
    if (Index == IH_CLK_CTRL)
    {
        if ((Value & (1U << IH_DBUS_MUX_CLK_SOFT_OVERRIDE_SHIFT)) != 0)
        {
            Value |= (1U << IH_IH_BUFFER_MEM_CLK_SOFT_OVERRIDE_SHIFT);
        }
    }
    FunctionCast(WrapIRQMGRWriteRegister, Singleton().orgIRQMGRWriteRegister)(CTX, Index, Value);
}

void *VanGogh::X6000FB::WrapCreateRegisterAccess(void *InitData)
{
    getMember<UInt32>(InitData, 0x24) = SMUIO_BASE_0 + ROM_INDEX;
    getMember<UInt32>(InitData, 0x28) = SMUIO_BASE_0 + ROM_DATA;
    return FunctionCast(WrapCreateRegisterAccess, Singleton().orgCreateRegisterAccess)(InitData);
}

IOReturn VanGogh::X6000FB::InitializeReservedVRAM(void *ThatIGuess)
{
    static constexpr IOOptionBits VramMappingOptions = kIOMapWriteCombineCache | kIOMapAnywhere;
    auto ReturnValue =
        Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor1_32bpp, 0, 0x40000, VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor1_2bpp, 0x40000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor2_32bpp, 0x80000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor2_2bpp, 0xC0000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor3_32bpp, 0x100000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor3_2bpp, 0x140000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor4_32bpp, 0x180000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor4_2bpp, 0x1C0000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor5_32bpp, 0x200000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor5_2bpp, 0x240000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor6_32bpp, 0x280000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorCursor6_2bpp, 0x2C0000, 0x40000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorPPLIBReserved, 0x300000, 0x100000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    ReturnValue = Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorDMCUBReserved, 0x400000, 0x100000,
        VramMappingOptions);
    if (ReturnValue != kIOReturnSuccess) { return ReturnValue; }
    return Singleton().orgMapMemorySubRange(ThatIGuess, KAMDReservedMemorySelectorReserveVRAM, 0, 0x500000,
        VramMappingOptions);
}