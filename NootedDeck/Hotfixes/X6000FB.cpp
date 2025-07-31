#include <Headers/kern_api.hpp>
#include <KextHeaders/Hotfixes/X6000FB.hpp>
#include <KextHeaders/NDeck.hpp>
#include <KextHeaders/Patcher+.hpp>

static const char *RadeonX6000FramebufferPath =
    "/System/Library/Extensions/AMDRadeonX6000Framebuffer.kext/Contents/MacOS/AMDRadeonX6000Framebuffer";

static KernelPatcher::KextInfo RadeonX6000FramebufferKext
{
    "com.apple.kext.AMDRadeonX6000Framebuffer",
    &RadeonX6000FramebufferPath,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};

// Patches

static const UInt8 KDpReceiverPowerCtrlPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53,
    0x48, 0x83, 0xEC, 0x10, 0x89, 0xF3, 0xB0, 0x02, 0x28, 0xD8};
static const UInt8 KDpReceiverPowerCtrlPattern1404[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54,
    0x53, 0x48, 0x83, 0xEC, 0x10, 0x41, 0x89, 0xF7, 0xB0, 0x02, 0x44, 0x28, 0xF8};

// Remove new FB count condition so we can restore the original behavior before Ventura.
static const UInt8 KControllerPowerUpOriginal[] = {0x38, 0xC8, 0x0F, 0x42, 0xC8, 0x88, 0x8F, 0xBC, 0x00, 0x00, 0x00,
    0x72, 0x00};
static const UInt8 KControllerPowerUpOriginalMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00};
static const UInt8 KControllerPowerUpReplace[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEB, 0x00};
static const UInt8 KControllerPowerUpReplaceMask[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00};

// Remove new problematic Ventura pixel clock multiplier calculation which causes timing validation mishaps.
static const UInt8 KValidateDetailedTimingOriginal[] = {0x66, 0x0F, 0x2E, 0xC1, 0x76, 0x06, 0xF2, 0x0F, 0x5E, 0xC1};
static const UInt8 KValidateDetailedTimingPatched[]  = {0x66, 0x0F, 0x2E, 0xC1, 0x66, 0x90, 0xF2, 0x0F, 0x5E, 0xC1};

// Module logic

static Hotfixes::X6000FB Instance {};

Hotfixes::X6000FB &Hotfixes::X6000FB::Singleton() { return Instance; }

void Hotfixes::X6000FB::StartModule()
{
    PANIC_COND(this->KextLoaded, "X6000FB", "Attempted to load module twice!");
    this->KextLoaded = true;

    SYSLOG("X6000FB", "Module loaded successfully!");

    lilu.onKextLoadForce(
        &RadeonX6000FramebufferKext, 1,
        [](void *user, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size) {
            static_cast<Hotfixes::X6000FB *>(user)->ProcessKext(Patcher, ID, Slide, Size);
        },
        this);
}

void Hotfixes::X6000FB::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    if (RadeonX6000FramebufferKext.loadIndex != ID) { return; }

    if (checkKernelArgument("-NDeckDPDelay"))
    {
        if (NDeck::Singleton().GetAttributes().IsSonoma1404Plus())
        {
            Patcher+::PatternRouteRequest Request {"_dp_receiver_power_ctrl", wrapDpReceiverPowerCtrl,
                this->OrgDpReceiverPowerCtrl, kDpReceiverPowerCtrlPattern1404};
            PANIC_COND(!Request.route(Patcher, ID, Slide, Size), "X6000FB",
                "Failed to route dp_receiver_power_ctrl (14.4+)");
        }
        else
        {
            Patcher+::PatternRouteRequest Request {"_dp_receiver_power_ctrl", wrapDpReceiverPowerCtrl,
                this->OrgDpReceiverPowerCtrl, kDpReceiverPowerCtrlPattern};
            PANIC_COND(!Request.route(Patcher, ID, Slide, Size), "X6000FB", "Failed to route dp_receiver_power_ctrl");
        }
    }

    if (NDeck::Singleton().GetAttributes().IsVenturaPlus())
    {
        Patcher+::PatternSolveRequest SolveRequest{
            "__ZNK34AMDRadeonX6000_AmdRadeonController18messageAcceleratorE25_eAMDAccelIOFBRequestTypePvS1_S1_",
            this->orgMessageAccelerator};
        PANIC_COND(!SolveRequest.solve(Patcher, ID, Slide, Size), "X6000FB", "Failed to resolve messageAccelerator");
    }

    if (NDeck::Singleton().GetAttributes().IsVenturaPlus())
    {
        Patcher+::PatternRouteRequest Request {"__ZN34AMDRadeonX6000_AmdRadeonController7powerUpEv",
            WrapControllerPowerUp, this->OrgControllerPowerUp};
        PANIC_COND(!Request.route(Patcher, ID, Slide, Size), "X6000FB", "Failed to route powerUp");
    }

    if (NDeck::Singleton().GetAttributes().IsVenturaPlus())
    {
        const Patcher+::MaskedLookupPatch Patches[] =
        {
            {&RadeonX6000FramebufferKext, KControllerPowerUpOriginal, KControllerPowerUpOriginalMask,
                KControllerPowerUpReplace, KControllerPowerUpReplaceMask, 1},
            {&RadeonX6000FramebufferKext, KValidateDetailedTimingOriginal, KValidateDetailedTimingPatched, 1},
        };
        PANIC_COND(!Patcher+::MaskedLookupPatch::ApplyAll(Patcher, Patches, Slide, Size), "X6000FB",
            "Failed to apply logic revert Patches");
    }
}

UInt32 Hotfixes::X6000FB::WrapControllerPowerUp(void *ThatIGuess)
{
    auto &MFlags = getMember<UInt8>(ThatIGuess, 0x5F18);
    auto send = (MFlags & 2) == 0;
    MFlags |= 4;    // All framebuffers enabled
    auto ret = FunctionCast(WrapControllerPowerUp, Singleton().OrgControllerPowerUp)(ThatIGuess);
    if (send) { Singleton().orgMessageAccelerator(ThatIGuess, IOFBRequestControllerEnabled, nullptr, nullptr, nullptr); }
    return ret;
}

void Hotfixes::X6000FB::wrapDpReceiverPowerCtrl(void *Link, bool PowerOn)
{
    FunctionCast(wrapDpReceiverPowerCtrl, Singleton().OrgDpReceiverPowerCtrl)(Link, PowerOn);
    IOSleep(250);
}
