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

static const UInt8 kCailAsicCapsTablePattern[] = {0x6E, 0x00, 0x00, 0x00, 0x98, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

static const UInt8 kPopulateVramInfoPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x53, 0x48, 0x81, 0xEC,
    0x08, 0x01, 0x00, 0x00, 0x40, 0x89, 0xF0, 0x40, 0x89, 0xF0, 0x4C, 0x8D, 0xBD, 0xE0, 0xFE, 0xFF, 0xFF};
static const UInt8 KPopulateVramInfoPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static const UInt8 kGetNumberOfConnectorsPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x40, 0x8B, 0x40, 0x28, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x85, 0x00, 0x74, 0x00};
static const UInt8 kGetNumberOfConnectorsPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

static const UInt8 kIH40IVRingInitHardwarePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x40, 0x89, 0xF0, 0x49, 0x89, 0xF0, 0x40, 0x8B, 0x00, 0x00, 0x44, 0x00, 0x00};
static const UInt8 kIH40IVRingInitHardwarePatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xF0, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

static const UInt8 kIRQMGRWriteRegisterPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x41, 0x89, 0xD6, 0x49, 0x89, 0xF7, 0x48, 0x89, 0xFB, 0x48, 0x8B, 0x87, 0xB0, 0x00, 0x00, 0x00,
    0x48, 0x85, 0xC0};
static const UInt8 kIRQMGRWriteRegisterPattern1404[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x89, 0xD3, 0x49, 0x89, 0xF7, 0x49, 0x89, 0xFE, 0x48, 0x8B, 0x87, 0xB0, 0x00, 0x00, 0x00,
    0x48, 0x85, 0xC0};

// Fix register read (0xD31 -> 0xD2F) and family ID (0x8F -> 0x8E).
static const UInt8 kPopulateDeviceInfoOriginal[] {0xBE, 0x31, 0x0D, 0x00, 0x00, 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00,
    0xC7, 0x43, 0x00, 0x8F, 0x00, 0x00, 0x00};
static const UInt8 kPopulateDeviceInfoMask[] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kPopulateDeviceInfoPatched[] {0xBE, 0x2F, 0x0D, 0x00, 0x00, 0xFF, 0x90, 0x40, 0x01, 0x00, 0x00, 0xC7,
    0x43, 0x00, 0x8E, 0x00, 0x00, 0x00};

// Neutralize `AmdAtomVramInfo` creation null check.
// We don't have this entry in our VBIOS.
static const UInt8 kAmdAtomVramInfoNullCheckOriginal[] = {0x48, 0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0,
    0x0F, 0x84, 0x89, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7B, 0x18};
static const UInt8 kAmdAtomVramInfoNullCheckPatched[] = {0x48, 0x89, 0x83, 0x90, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x90, 0x48, 0x8B, 0x7B, 0x18};

// Ditto
static const UInt8 kAmdAtomVramInfoNullCheckOriginal1015[] = {0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00, 0x48, 0x85,
    0xC0, 0x74, 0x00};
static const UInt8 kAmdAtomVramInfoNullCheckOriginalMask1015[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00};
static const UInt8 kAmdAtomVramInfoNullCheckPatched1015[] = {0x48, 0x89, 0x83, 0x80, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x90};

// Neutralize `AmdAtomPspDirectory` creation null check.
// We don't have this entry in our VBIOS.
static const UInt8 kAmdAtomPspDirectoryNullCheckOriginal[] = {0x48, 0x89, 0x83, 0x88, 0x00, 0x00, 0x00, 0x48, 0x85,
    0xC0, 0x0F, 0x84, 0xA1, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7B, 0x18};
static const UInt8 kAmdAtomPspDirectoryNullCheckPatched[] = {0x48, 0x89, 0x83, 0x88, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x90, 0x48, 0x8B, 0x7B, 0x18};

// Neutralize `AmdAtomVramInfo` null check.
static const UInt8 kGetFirmwareInfoNullCheckOriginal[] = {0x48, 0x83, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x84,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x89};
static const UInt8 kGetFirmwareInfoNullCheckOriginalMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static const UInt8 kGetFirmwareInfoNullCheckPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x90,
    0x66, 0x90, 0x66, 0x90, 0x00, 0x00};
static const UInt8 kGetFirmwareInfoNullCheckPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};
static const UInt8 kGetFirmwareInfoNullCheckOriginal1404[] = {0x49, 0x83, 0xBC, 0x24, 0x90, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x49, 0x89};
static const UInt8 kGetFirmwareInfoNullCheckOriginalMask1404[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
static const UInt8 kGetFirmwareInfoNullCheckPatched1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x00, 0x00};
static const UInt8 kGetFirmwareInfoNullCheckPatchedMask1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00};

// Tell AGDC that we're an iGPU.
static const UInt8 kGetVendorInfoOriginal[] = {0x48, 0x00, 0x02, 0x10, 0x00, 0x00, 0x02};
static const UInt8 kGetVendorInfoMask[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const UInt8 kGetVendorInfoPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static const UInt8 kGetVendorInfoPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const UInt8 kGetVendorInfoOriginal1404[] = {0xC7, 0x00, 0x24, 0x02, 0x10, 0x00, 0x00, 0xC7, 0x00, 0x28, 0x02,
    0x00, 0x00, 0x00};
static const UInt8 kGetVendorInfoMask1404[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF};
static const UInt8 kGetVendorInfoPatched1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00};
static const UInt8 kGetVendorInfoPatchedMask1404[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00};

// Remove check for Navi family
static const UInt8 kInitializeDmcubServices1Original[] = {0x81, 0x79, 0x2C, 0x8F, 0x00, 0x00, 0x00};
static const UInt8 kInitializeDmcubServices1Patched[] = {0x39, 0xC0, 0x66, 0x90, 0x66, 0x90, 0x90};

// Set DMCUB ASIC constant to DCN 2.1
static const UInt8 kInitializeDmcubServices2Original[] = {0x83, 0xC0, 0xC4, 0x83, 0xF8, 0x0A, 0xB8, 0x03, 0x00, 0x00,
    0x00, 0x83, 0xD0, 0x00};
static const UInt8 kInitializeDmcubServices2Patched[] = {0xB8, 0x02, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x90};

// Ditto, 14.4+
static const UInt8 kInitializeDmcubServices2Original1404[] = {0x83, 0xC0, 0xC4, 0x31, 0xC9, 0x83, 0xF8, 0x0A, 0x83,
    0xD1, 0x03};
static const UInt8 kInitializeDmcubServices2Patched1404[] = {0xB9, 0x02, 0x00, 0x00, 0x00, 0x66, 0x90, 0x66, 0x90, 0x66,
    0x90};

/* // Ditto, 10.15
static const UInt8 kInitializeDmcubServices2Original1015[] = {0xC7, 0x46, 0x20, 0x01, 0x00, 0x00, 0x00};
static const UInt8 kInitializeDmcubServices2Patched1015[] = {0xC7, 0x46, 0x20, 0x02, 0x00, 0x00, 0x00};

// 10.15: Set inst_const_size/bss_data_size to 0. To disable DMCUB firmware loading logic.
static const UInt8 kInitializeHardware1Original[] = {0x49, 0xBC, 0x00, 0x0A, 0x01, 0x00, 0xF4, 0x01, 0x00, 0x00};
static const UInt8 kInitializeHardware1Patched[] = {0x49, 0xC7, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90};

// 10.15: Set fw_inst_const to nullptr, pt.2 of above.
static const UInt8 kInitializeHardware2Original[] = {0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x00, 0x10,
    0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C};
static const UInt8 kInitializeHardware2OriginalMask[] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};
static const UInt8 kInitializeHardware2Patched[] = {0x49, 0xC7, 0xC5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kInitializeHardware2PatchedMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// 10.15: Disable DMCUB firmware loading from DAL. HWLibs should be doing that.
static const UInt8 kAmdDalServicesInitializeOriginal[] = {0xBE, 0x01, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x00, 0x00, 0x60};
static const UInt8 kAmdDalServicesInitializeOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0x00, 0x00, 0xFF};
static const UInt8 kAmdDalServicesInitializePatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};
static const UInt8 kAmdDalServicesInitializePatchedMask[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00}; */ // Catalina Logic

/* // Raven: Change cursor and underflow tracker count to 4 instead of 6.
static const UInt8 kCreateControllerServicesOriginal[] = {0x40, 0x00, 0x00, 0x40, 0x83, 0x00, 0x06};
static const UInt8 kCreateControllerServicesOriginalMask[] = {0xF0, 0x00, 0x00, 0xF0, 0xFF, 0x00, 0xFF};
static const UInt8 kCreateControllerServicesPatched[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
static const UInt8 kCreateControllerServicesPatchedMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F};        // Commented out Raven logic for now, might or might not need later

// Ditto, 10.15.
static const UInt8 kCreateControllerServicesOriginal1015[] = {0x48, 0x00, 0x00, 0x48, 0x83, 0x00, 0x05};
static const UInt8 kCreateControllerServicesOriginalMask1015[] = {0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF};
static const UInt8 kCreateControllerServicesPatched1015[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
static const UInt8 kCreateControllerServicesPatchedMask1015[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F}; */ // More catalina shiz

/* // Raven: Change cursor count to 4 instead of 6.
static const UInt8 kSetupCursorsOriginal[] = {0x40, 0x83, 0x00, 0x05};
static const UInt8 kSetupCursorsOriginalMask[] = {0xF0, 0xFF, 0x00, 0xFF};
static const UInt8 kSetupCursorsPatched[] = {0x00, 0x00, 0x00, 0x03};
static const UInt8 kSetupCursorsPatchedMask[] = {0x00, 0x00, 0x00, 0x0F}; */ // Commented out Raven logic for now, might or might not need later

// Ditto, 12.0+.
static const UInt8 kSetupCursorsOriginal12[] = {0x40, 0x83, 0x00, 0x06};
static const UInt8 kSetupCursorsOriginalMask12[] = {0xF0, 0xFF, 0x00, 0xFF};
static const UInt8 kSetupCursorsPatched12[] = {0x00, 0x00, 0x00, 0x04};
static const UInt8 kSetupCursorsPatchedMask12[] = {0x00, 0x00, 0x00, 0x0F};

/* // Raven: Change link count to 4 instead of 6.
static const UInt8 kCreateLinksOriginal[] = {0x06, 0x00, 0x00, 0x00, 0x40};
static const UInt8 kCreateLinksOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xF0};
static const UInt8 kCreateLinksPatched[] = {0x04, 0x00, 0x00, 0x00, 0x00};
static const UInt8 kCreateLinksPatchedMask[] = {0x0F, 0x00, 0x00, 0x00, 0x00}; */ // Commented out Raven logic for now, might or might not need later

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
        [](void *user, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
        {
            static_cast<VanGogh::X6000FB *>(user)->ProcessKext(Patcher, ID, Slide, Size);
        },
    this);
}

void VanGogh::X6000FB::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    if (RadeonX6000FramebufferKext.loadIndex != ID) { return; }

    NDeck::Singleton().InitHWLate();

    CAILAsicCapsEntry *OrgAsicCapsTable = nullptr;
    PatcherPlus::PatternSolveRequest cailAsicCapsSolveRequest {"__ZL20CAIL_ASIC_CAPS_TABLE", OrgAsicCapsTable,
        kCailAsicCapsTablePattern};
    PANIC_COND(!cailAsicCapsSolveRequest.solve(Patcher, ID, Slide, Size), "X6000FB",
        "Failed to resolve CAIL_ASIC_CAPS_TABLE");

    PatcherPlus::PatternRouteRequest Requests[] =
    {
        {"__ZNK15AmdAtomVramInfo16populateVramInfoER16AtomFirmwareInfo", PopulateVramInfo, kPopulateVramInfoPattern,
            KPopulateVramInfoPatternMask},
        {"__ZNK32AMDRadeonX6000_AmdAsicInfoNavi1027getEnumeratedRevisionNumberEv", getEnumeratedRevision},
        {"__ZNK22AmdAtomObjectInfo_V1_421getNumberOfConnectorsEv", wrapGetNumberOfConnectors,
            this->orgGetNumberOfConnectors, kGetNumberOfConnectorsPattern, kGetNumberOfConnectorsPatternMask},
    };
    PANIC_COND(!PatcherPlus::PatternRouteRequest::RouteAll(Patcher, ID, Requests, Slide, Size), "X6000FB",
        "Failed to route symbols");

    if (NDeck::Singleton().GetAttributes().isBigSurAndLater()) {
        KernelPatcher::RouteRequest Request {
            "__ZN32AMDRadeonX6000_AmdRegisterAccess20createRegisterAccessERNS_8InitDataE", wrapCreateRegisterAccess,
            this->orgCreateRegisterAccess};
        PANIC_COND(!Patcher.routeMultiple(ID, &Request, 1, Slide, Size), "X6000FB",
            "Failed to route createRegisterAccess");
    }

    if (NDeck::Singleton().GetAttributes().isRenoir())
    {
        this->orgMapMemorySubRange = Patcher.solveSymbol<mapMemorySubRange_t>(ID,
            "__ZN37AMDRadeonX6000_AmdDeviceMemoryManager17mapMemorySubRangeE25AmdReservedMemorySelectoryyj", Slide,
            Size, true);
        PANIC_COND(this->orgMapMemorySubRange == nullptr, "X6000FB", "Failed to solve mapMemorySubRange");
        PatcherPlus::PatternRouteRequest Requests[] = {
            {"_IH_4_0_IVRing_InitHardware", wrapIH40IVRingInitHardware, this->orgIH40IVRingInitHardware,
                kIH40IVRingInitHardwarePattern, kIH40IVRingInitHardwarePatternMask},
            {"__ZN41AMDRadeonX6000_AmdDeviceMemoryManagerNavi21intializeReservedVramEv", intializeReservedVram},
        };
        PANIC_COND(!PatcherPlus::PatternRouteRequest::RouteAll(Patcher, ID, Requests, Slide, Size), "X6000FB",
            "Failed to route IH_4_0_IVRing_InitHardware and intializeReservedVram");
        if (NDeck::Singleton().GetAttributes().isSonoma1404Plus())
        {
            PatcherPlus::PatternRouteRequest Request {"_IRQMGR_WriteRegister", wrapIRQMGRWriteRegister,
                this->orgIRQMGRWriteRegister, kIRQMGRWriteRegisterPattern1404};
            PANIC_COND(!Request.Route(Patcher, ID, Slide, Size), "X6000FB",
                "Failed to route IRQMGR_WriteRegister (14.4+)");
        }
        else
        {
            PatcherPlus::PatternRouteRequest Request {"_IRQMGR_WriteRegister", wrapIRQMGRWriteRegister,
                this->orgIRQMGRWriteRegister, kIRQMGRWriteRegisterPattern};
            PANIC_COND(!Request.Route(Patcher, ID, Slide, Size), "X6000FB", "Failed to route IRQMGR_WriteRegister");
        }
    }

    const PatcherPlus::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kPopulateDeviceInfoOriginal,
        kPopulateDeviceInfoMask, kPopulateDeviceInfoPatched, kPopulateDeviceInfoMask, 1};
    PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB", "Failed to apply populateDeviceInfo Patch");

    if (NDeck::Singleton().GetAttributes().isSonoma1404Plus()) {
        const PatcherPlus::MaskedLookupPatch patches[] = {
            {&RadeonX6000FramebufferKext, kGetFirmwareInfoNullCheckOriginal1404,
                kGetFirmwareInfoNullCheckOriginalMask1404, kGetFirmwareInfoNullCheckPatched1404,
                kGetFirmwareInfoNullCheckPatchedMask1404, 1},
            {&RadeonX6000FramebufferKext, kGetVendorInfoOriginal1404, kGetVendorInfoMask1404, kGetVendorInfoPatched1404,
                kGetVendorInfoPatchedMask1404, 1},
        };
        PANIC_COND(!PatcherPlus::MaskedLookupPatch::applyAll(Patcher, patches, Slide, Size), "X6000FB",
            "Failed to apply patches (14.4)");
    } else {
        const PatcherPlus::MaskedLookupPatch patches[] = {
            {&RadeonX6000FramebufferKext, kGetFirmwareInfoNullCheckOriginal, kGetFirmwareInfoNullCheckOriginalMask,
                kGetFirmwareInfoNullCheckPatched, kGetFirmwareInfoNullCheckPatchedMask, 1},
            {&RadeonX6000FramebufferKext, kGetVendorInfoOriginal, kGetVendorInfoMask, kGetVendorInfoPatched,
                kGetVendorInfoPatchedMask, 1},
        };
        PANIC_COND(!PatcherPlus::MaskedLookupPatch::applyAll(Patcher, patches, Slide, Size), "X6000FB",
            "Failed to apply patches");
    }

    if (NDeck::Singleton().GetAttributes().isCatalina()) {
        const PatcherPlus::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kAmdAtomVramInfoNullCheckOriginal1015,
            kAmdAtomVramInfoNullCheckOriginalMask1015, kAmdAtomVramInfoNullCheckPatched1015, 1};
        PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB", "Failed to apply null check Patch");
    } else {
        const PatcherPlus::MaskedLookupPatch patches[] = {
            {&RadeonX6000FramebufferKext, kAmdAtomVramInfoNullCheckOriginal, kAmdAtomVramInfoNullCheckPatched, 1},
            {&RadeonX6000FramebufferKext, kAmdAtomPspDirectoryNullCheckOriginal, kAmdAtomPspDirectoryNullCheckPatched,
                1},
        };
        PANIC_COND(!PatcherPlus::MaskedLookupPatch::applyAll(Patcher, patches, Slide, Size), "X6000FB",
            "Failed to apply null check patches");
    }

    if (NDeck::Singleton().GetAttributes().isRenoir())
    {
        const PatcherPlus::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kInitializeDmcubServices1Original,
            kInitializeDmcubServices1Patched, 1};
        PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB",
            "Failed to apply initializeDmcubServices family ID Patch");
        if (NDeck::Singleton().GetAttributes().isCatalina())
        {
            const PatcherPlus::MaskedLookupPatch patches[] = {
                {&RadeonX6000FramebufferKext, kInitializeDmcubServices2Original1015,
                    kInitializeDmcubServices2Patched1015, 1},
                {&RadeonX6000FramebufferKext, kInitializeHardware1Original, kInitializeHardware1Patched, 1},
                {&RadeonX6000FramebufferKext, kInitializeHardware2Original, kInitializeHardware2OriginalMask,
                    kInitializeHardware2Patched, kInitializeHardware2PatchedMask, 1},
                {&RadeonX6000FramebufferKext, kAmdDalServicesInitializeOriginal, kAmdDalServicesInitializeOriginalMask,
                    kAmdDalServicesInitializePatched, kAmdDalServicesInitializePatchedMask, 1},
            };
            PANIC_COND(!PatcherPlus::MaskedLookupPatch::applyAll(Patcher, patches, Slide, Size), "X6000FB",
                "Failed to apply AmdDalDmcubService and AmdDalServices::initialize patches (10.15)");
        } else if (NDeck::Singleton().GetAttributes().isSonoma1404Plus()) {
            const PatcherPlus::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext,
                kInitializeDmcubServices2Original1404, kInitializeDmcubServices2Patched1404, 1};
            PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB",
                "Failed to apply initializeDmcubServices ASIC Patch (14.4+)");
        } else {
            const PatcherPlus::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kInitializeDmcubServices2Original,
                kInitializeDmcubServices2Patched, 1};
            PANIC_COND(!Patch.apply(Patcher, Slide, Size), "X6000FB",
                "Failed to apply initializeDmcubServices ASIC Patch");
        }
    }

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
    // We need to Patch the kext to create only 4 cursors, links and underflow trackers.
    
    // Actually no we do not because this is Van Gogh which uses DCN 3 Lolz!
}