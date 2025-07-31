// NootedDeck
// 🛈 brurmonemt 2025
// © ChefKiss 2024-25
// 
// File: NootedDeck/Backlight.cpp
// handle backlighting (only needed for LCD steam decks and just LCD displays in general)

#include <Headers/kern_api.hpp>
#include <Headers/kern_devinfo.hpp>
#include <KextHeaders/Backlight.hpp>
#include <KextHeaders/NDeck.hpp>

static const char *RadeonX6000Framebuffer =
    "/System/Library/Extensions/AMDRadeonX6000Framebuffer.kext/Contents/MacOS/AMDRadeonX6000Framebuffer";

static KernelPatcher::KextInfo RadeonX6000FramebufferKext {
    "com.apple.kext.AMDRadeonX6000Framebuffer",
    &RadeonX6000Framebuffer,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};

static const char *BacklightPath   = "/System/Library/Extensions/AppleBacklight.kext/Contents/MacOS/AppleBacklight";
static const char *MCCSControlPath = "/System/Library/Extensions/AppleMCCSControl.kext/Contents/MacOS/AppleMCCSControl";

static KernelPatcher::KextInfo BacklightKext {
    "com.apple.driver.AppleBacklight",
    &BacklightPath,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};
static KernelPatcher::KextInfo MCCSControlKext {
    "com.apple.driver.AppleMCCSControl",
    &MCCSControlPath,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};

// Patterns

static const UInt8 KDcePanelCntlHwInitPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x49, 0x89, 0xFD, 0x4C, 0x8D, 0x45, 0xD4, 0x41, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00};
static const UInt8 KDcePanelCntlHwInitPattern1404[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53,
    0x48, 0x83, 0xEC, 0x10, 0x48, 0x89, 0xFB, 0x4C, 0x8D, 0x45, 0xDC, 0x41, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00};

static const UInt8 KDceDriverSetBacklightPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
    0x54, 0x53, 0x50, 0x41, 0x89, 0xF0, 0x40, 0x89, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x7F, 0x08, 0x40, 0x8B, 0x40, 0x28, 0x8B, 0x70, 0x10};
static const UInt8 KDceDriverSetBacklightPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF,
    0xFF};

static const UInt8 KLinkCreatePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x53,
    0x48, 0x81, 0xEC, 0x00, 0x03, 0x00, 0x00, 0x49, 0x89, 0xFD, 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B,
    0x00, 0x48, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 0x00};
static const UInt8 KLinkCreatePatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};

static const UInt8 KDcLinkSetBacklightLevelPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x41, 0x89, 0xD6, 0x41, 0x89, 0xF4};
static const UInt8 KDcLinkSetBacklightLevelPattern1404[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55,
    0x41, 0x54, 0x53, 0x50, 0x89, 0xD3, 0x41, 0x89, 0xF6, 0x49, 0x89, 0xFC};

static const UInt8 KDcLinkSetBacklightLevelNitsPattern[] = {0x55, 0x48, 0x89, 0xE5, 0x53, 0x50, 0x40, 0x88, 0x75, 0x00,
    0x48, 0x85, 0xFF, 0x74, 0x00};
static const UInt8 KDcLinkSetBacklightLevelNitsPatternMask[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

// Module logic

constexpr UInt32 FBAttributeBacklight = static_cast<UInt32>('bklt');

static Backlight Instance {};

Backlight &Backlight::Singleton() { return Instance; }

void Backlight::StartModule()
{
    PANIC_COND(this->ModuleLoaded, "Backlight", "Attempted to load module twice!");
    this->ModuleLoaded = true;

    bool BacklightArg = false;
    if (BaseDeviceInfo::get().modelType != WIOKit::ComputerModel::ComputerLaptop &&
        (!PE_parse_boot_argn("AMDBacklight", &BacklightArg, sizeof(BacklightArg)) || !BacklightArg))
    {
        SYSLOG("Backlight", "Module disabled");
        return;
    }

    switch (getKernelVersion())
    {
        case KernelVersion::Catalina:
            this->DcLinkCapabillsField = 0x1EA;
            break;
        case KernelVersion::BigSur:
            this->DcLinkCapabillsField = 0x26C;
            break;
        case KernelVersion::Monterey:
            this->DcLinkCapabillsField = 0x284;
            break;
        case KernelVersion::Ventura:
        case KernelVersion::Sonoma:
        case KernelVersion::Sequoia:
        case KernelVersion::Tahoe:
            this->DcLinkCapabillsField = 0x28C;
            break;
        default:
            PANIC("Backlight", "Unsupported kernel version %d", getKernelVersion());
    }

    SYSLOG("NDeck", "Module loaded successfully!")

    lilu.onKextLoadForce(&RadeonX6000FramebufferKext);
    lilu.onKextLoadForce(&BacklightKext);
    lilu.onKextLoadForce(&MCCSControlKext);
    lilu.onPatcherLoadForce(
        [](void *user, KernelPatcher &) { static_cast<Backlight *>(user)->registerDispMaxBrightnessNotif(); }, this);
    lilu.onKextLoadForce(
        nullptr, 0,
        [](void *user, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
        {
            static_cast<Backlight *>(user)->ProcessKext(Patcher, ID, Slide, Size);
        },
    this);
}

void Backlight::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t size)
{
    if (RadeonX6000FramebufferKext.loadIndex == ID)
    {
        if (NDeck::Singleton().getAttributes().IsSonoma1404Plus())
        {
            Patcher+::PatternSolveRequest solveRequest {"_dc_link_set_backlight_level",
                this->OrgDcLinkSetBacklightLevel, KDcLinkSetBacklightLevelPattern1404};
            PANIC_COND(!solveRequest.solve(Patcher, ID, Slide, size), "Backlight",
                "Failed to resolve dc_link_set_backlight_level");
        }
        else
        {
            Patcher+::PatternSolveRequest solveRequest {"_dc_link_set_backlight_level",
                this->OrgDcLinkSetBacklightLevel, KDcLinkSetBacklightLevelPattern};
            PANIC_COND(!solveRequest.solve(Patcher, ID, Slide, size), "Backlight",
                "Failed to resolve dc_link_set_backlight_level");
        }

        Patcher+::PatternSolveRequest solveRequest {"_dc_link_set_backlight_level_nits",
            this->OrgDcLinkSetBacklightLevelNits, KDcLinkSetBacklightLevelNitsPattern,
            KDcLinkSetBacklightLevelNitsPatternMask};
        PANIC_COND(!solveRequest.solve(Patcher, ID, Slide, size), "Backlight",
            "Failed to resolve dc_link_set_backlight_level_nits");
        Patcher+::PatternRouteRequest Requests[] =
        {
            {"_link_create", WrapLinkCreate, this->OrgLinkCreate, kLinkCreatePattern, kLinkCreatePatternMask},
            {"__ZN35AMDRadeonX6000_AmdRadeonFramebuffer25setAttributeForConnectionEijm", wrapSetAttributeForConnection,
                this->OrgSetAttributeForConnection},
            {"__ZN35AMDRadeonX6000_AmdRadeonFramebuffer25getAttributeForConnectionEijPm", wrapGetAttributeForConnection,
                this->OrgGetAttributeForConnection},
        };
        PANIC_COND(!Patcher+::PatternRouteRequest::RouteAll(Patcher, ID, Requests, Slide, size), "Backlight",
            "Failed to route backlight symbols");
    }
    else if (BacklightKext.loadIndex == ID)
    {
        KernelPatcher::RouteRequest Request {"__ZN15AppleIntelPanel10setDisplayEP9IODisplay", WrapApplePanelSetDisplay,
            orgApplePanelSetDisplay};
        if (Patcher.routeMultiple(BacklightKext.loadIndex, &Request, 1, Slide, size))
        {
            static const UInt8 find[] = "F%uT%04x";
            static const UInt8 replace[] = "F%uTxxxx";
            const Patcher+::MaskedLookupPatch patch {&BacklightKext, find, replace, 1};
            SYSLOG_COND(!patch.apply(Patcher, Slide, size), "Backlight", "Failed to apply backlight patch");
        }
        Patcher.clearError();
    }
    else if (MCCSControlKext.loadIndex == ID)
    {
        KernelPatcher::RouteRequest Requests[] =
        {
            {"__ZN25AppleMCCSControlGibraltar5probeEP9IOServicePi", Return0},
            {"__ZN21AppleMCCSControlCello5probeEP9IOServicePi", Return0},
        };
        Patcher.routeMultiple(ID, Requests, Slide, size);
        Patcher.clearError();
    }
}

bool Backlight::OnAppleBacklightDisplayLoad(void *, void *, IOService *NewService, IONotifier *)
{
    OSDictionary *params = OSDynamicCast(OSDictionary, NewService->getProperty("IODisplayParameters"));
    if (params == nullptr)
    {
        DBGLOG("Backlight", "%s: No 'IODisplayParameters' property", __FUNCTION__);
        return false;
    }

    OSDictionary *LinearBrightness = OSDynamicCast(OSDictionary, params->getObject("linear-brightness"));
    if (LinearBrightness == nullptr)
    {
        DBGLOG("Backlight", "%s: No 'linear-brightness' property", __FUNCTION__);
        return false;
    }

    OSNumber *MaxBrightness = OSDynamicCast(OSNumber, LinearBrightness->getObject("max"));
    if (MaxBrightness == nullptr)
    {
        DBGLOG("Backlight", "%s: No 'max' property", __FUNCTION__);
        return false;
    }

    if (MaxBrightness->unsigned32BitValue() == 0)
    {
        DBGLOG("Backlight", "%s: 'max' property is 0", __FUNCTION__);
        return false;
    }

    Singleton().MaxPwmBacklightLevel = MaxBrightness->unsigned32BitValue();
    DBGLOG("Backlight", "%s: Max brightness: 0x%X", __FUNCTION__, Singleton().MaxPwmBacklightLevel);

    return true;
}

void Backlight::registerDispMaxBrightnessNotif()
{
    if (this->DisplayNotif != nullptr) { return; }

    auto *Matching = IOService::serviceMatching("AppleBacklightDisplay");
    if (Matching == nullptr)
    {
        SYSLOG("Backlight", "%s: Failed to create match dictionary", __FUNCTION__);
        return;
    }

    this->DisplayNotif =
        IOService::addMatchingNotification(gIOFirstMatchNotification, Matching, OnAppleBacklightDisplayLoad, nullptr);
    SYSLOG_COND(this->DisplayNotif == nullptr, "Backlight", "%s: Failed to register notification", __FUNCTION__);
    OSSafeReleaseNULL(Matching);
}

IOReturn Backlight::wrapSetAttributeForConnection(IOService *Framebuffer, IOIndex ConnectIndex, IOSelect Attribute,
    uintptr_t Value)
{
    auto ReturnValue = FunctionCast(wrapSetAttributeForConnection, Singleton().OrgSetAttributeForConnection)(Framebuffer,
        ConnectIndex, Attribute, Value);
    if (Attribute != FbAttributeBacklight) { return ReturnValue; }

    Singleton().CurrentPwmBacklightLevel = static_cast<UInt32>(Value);

    if (Singleton().MaxPwmBacklightLevel == 0) { return kIOReturnInternalError; }

    UInt32 percentage = Singleton().CurrentPwmBacklightLevel * 100 / Singleton().MaxPwmBacklightLevel;

    if (Singleton().SupportsAUX)
    {
        // TODO: Obtain the actual max brightness for the screen
        UInt32 AuxValue = (Singleton().maxOLED * percentage) / 100;
        // dc_link_set_backlight_level_nits doesn't print the new backlight level, so we'll do it
        DBGLOG("Backlight", "%s: New AUX brightness: %d millinits (%d nits)", __FUNCTION__, AuxValue,
            (AuxValue / 1000));
        Singleton().OrgDcLinkSetBacklightLevelNits(Singleton().EmbeddedPanelLink, true, AuxValue, 15000);
    }
    else
    {
        UInt32 PwmValue = (percentage * 0xFFFF) / 100;
        DBGLOG("Backlight", "%s: New PWM brightness: 0x%X", __FUNCTION__, PwmValue);
        if (Singleton().OrgDcLinkSetBacklightLevel(Singleton().EmbeddedPanelLink, PwmValue, 0))
        {
            return KIOReturnSuccess;
        }
    }

    return kIOReturnDeviceError;
}

IOReturn Backlight::wrapGetAttributeForConnection(IOService *Framebuffer, IOIndex ConnectIndex, IOSelect Attribute,
    uintptr_t *Value)
{
    auto ReturnValue = FunctionCast(wrapGetAttributeForConnection, Singleton().OrgGetAttributeForConnection)(Framebuffer,
        ConnectIndex, Attribute, Value);
    if (Attribute != FbAttributeBacklight) { return ReturnValue; }
    *Value = Singleton().CurrentPwmBacklightLevel;
    return KIOReturnSuccess;
}

UInt32 Backlight::wrapDcePanelCntlHwInit(void *PanelControl)
{
    Singleton().PanelControlPtr = PanelControl;
    return FunctionCast(wrapDcePanelCntlHwInit, Singleton().OrgDcePanelCntlHwInit)(PanelControl);
}

void *Backlight::WrapLinkCreate(void *Data)
{
    void *ReturnValue = FunctionCast(WrapLinkCreate, Singleton().OrgLinkCreate)(Data);

    if (ReturnValue == nullptr) { return nullptr; }

    auto SignalType = getMember<UInt32>(ReturnValue, 0x38);
    switch (SignalType)
    {
        case SIGNAL_TYPE_LVDS: {
            if (Singleton().EmbeddedPanelLink != nullptr)
            {
                SYSLOG("Backlight", "EMBEDDED PANEL LINK WAS ALREADY SET AND DISCOVERED NEW ONE!!!!");
                SYSLOG("Backlight", "REPORT THIS TO THE DEVELOPERS AS SOON AS POSSIBLE!!!!");
            }
            Singleton().EmbeddedPanelLink = ReturnValue;
            DBGLOG("Backlight", "Will use DMCU for Display brightness control.");
        }
        case SIGNAL_TYPE_EDP: {
            if (Singleton().EmbeddedPanelLink != nullptr)
            {
                SYSLOG("Backlight", "EMBEDDED PANEL LINK WAS ALREADY SET AND DISCOVERED NEW ONE!!!!");
                SYSLOG("Backlight", "REPORT THIS TO THE DEVELOPERS AS SOON AS POSSIBLE!!!!");
            }
            Singleton().EmbeddedPanelLink = ReturnValue;
            Singleton().SupportsAUX = (Singleton().DcLinkCapabillsField.get(ReturnValue) & DC_DPCD_EXT_CAPS_OLED) != 0;

            DBGLOG("Backlight", "Will use %s for Display brightness control.",
                Singleton().SupportsAUX ? "AUX" : "DMCU");
        }
        default: {
            break;
        }
    }

    return ReturnValue;
}

struct ApplePanelData {
    const char *DeviceName;
    UInt8 DeviceData[36];
};

static ApplePanelData AppleBacklightData[] =
{
    {"F14Txxxx", {0x00, 0x11, 0x00, 0x00, 0x00, 0x34, 0x00, 0x52, 0x00, 0x73, 0x00, 0x94, 0x00, 0xBE, 0x00, 0xFA, 0x01,
                     0x36, 0x01, 0x72, 0x01, 0xC5, 0x02, 0x2F, 0x02, 0xB9, 0x03, 0x60, 0x04, 0x1A, 0x05, 0x0A, 0x06,
                     0x0E, 0x07, 0x10}},
    {"F15Txxxx", {0x00, 0x11, 0x00, 0x00, 0x00, 0x36, 0x00, 0x54, 0x00, 0x7D, 0x00, 0xB2, 0x00, 0xF5, 0x01, 0x49, 0x01,
                     0xB1, 0x02, 0x2B, 0x02, 0xB8, 0x03, 0x59, 0x04, 0x13, 0x04, 0xEC, 0x05, 0xF3, 0x07, 0x34, 0x08,
                     0xAF, 0x0A, 0xD9}},
    {"F16Txxxx", {0x00, 0x11, 0x00, 0x00, 0x00, 0x18, 0x00, 0x27, 0x00, 0x3A, 0x00, 0x52, 0x00, 0x71, 0x00, 0x96, 0x00,
                     0xC4, 0x00, 0xFC, 0x01, 0x40, 0x01, 0x93, 0x01, 0xF6, 0x02, 0x6E, 0x02, 0xFE, 0x03, 0xAA, 0x04,
                     0x78, 0x05, 0x6C}},
    {"F17Txxxx", {0x00, 0x11, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x34, 0x00, 0x4F, 0x00, 0x71, 0x00, 0x9B, 0x00, 0xCF, 0x01,
                     0x0E, 0x01, 0x5D, 0x01, 0xBB, 0x02, 0x2F, 0x02, 0xB9, 0x03, 0x60, 0x04, 0x29, 0x05, 0x1E, 0x06,
                     0x44, 0x07, 0xA1}},
    {"F18Txxxx", {0x00, 0x11, 0x00, 0x00, 0x00, 0x53, 0x00, 0x8C, 0x00, 0xD5, 0x01, 0x31, 0x01, 0xA2, 0x02, 0x2E, 0x02,
                     0xD8, 0x03, 0xAE, 0x04, 0xAC, 0x05, 0xE5, 0x07, 0x59, 0x09, 0x1C, 0x0B, 0x3B, 0x0D, 0xD0, 0x10,
                     0xEA, 0x14, 0x99}},
    {"F19Txxxx", {0x00, 0x11, 0x00, 0x00, 0x02, 0x8F, 0x03, 0x53, 0x04, 0x5A, 0x05, 0xA1, 0x07, 0xAE, 0x0A, 0x3D, 0x0E,
                     0x14, 0x13, 0x74, 0x1A, 0x5E, 0x24, 0x18, 0x31, 0xA9, 0x44, 0x59, 0x5E, 0x76, 0x83, 0x11, 0xB6,
                     0xC7, 0xFF, 0x7B}},
    {"F24Txxxx", {0x00, 0x11, 0x00, 0x01, 0x00, 0x34, 0x00, 0x52, 0x00, 0x73, 0x00, 0x94, 0x00, 0xBE, 0x00, 0xFA, 0x01,
                     0x36, 0x01, 0x72, 0x01, 0xC5, 0x02, 0x2F, 0x02, 0xB9, 0x03, 0x60, 0x04, 0x1A, 0x05, 0x0A, 0x06,
                     0x0E, 0x07, 0x10}},
};

size_t Backlight::Return0() { return 0; }

bool Backlight::WrapApplePanelSetDisplay(IOService *ThatIGuess, IODisplay *Display)
{
    static bool once = false;
    if (!once)
    {
        once = true;
        auto *panels = OSDynamicCast(OSDictionary, ThatIGuess->getProperty("ApplePanels"));
        if (panels)
        {
            auto *rawPanels = panels->copyCollection();
            panels = OSDynamicCast(OSDictionary, rawPanels);

            if (panels)
            {
                for (auto &entry : AppleBacklightData)
                {
                    auto pd = OSData::withBytes(entry.DeviceData, sizeof(entry.DeviceData));
                    if (pd)
                    {
                        panels->setObject(entry.DeviceName, pd);
                        // No release required by current AppleBacklight implementation.
                    }
                    else
                    {
                        SYSLOG("Backlight", "setDisplay: Cannot allocate data for %s", entry.DeviceName);
                    }
                }
                ThatIGuess->setProperty("ApplePanels", panels);
            }

            OSSafeReleaseNULL(rawPanels);
        }
        else
        {
            SYSLOG("Backlight", "setDisplay: Missing ApplePanels property");
        }
    }

    bool ReturnValue = FunctionCast(WrapApplePanelSetDisplay, Singleton().orgApplePanelSetDisplay)(ThatIGuess, Display);
    DBGLOG("Backlight", "setDisplay >> %d", ReturnValue);
    return ReturnValue;
}