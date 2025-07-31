// NootedDeck
// 🛈 brurmonemt 2025
// 
// File: NootedDeck/NDeck.cpp
// backlight class header

#pragma once
#include <Headers/kern_patcher.hpp>
#include <IOKit/IOService.h>
#include <IOKit/graphics/IOFramebuffer.h>
#include <IOKit/graphics/IOGraphicsTypes.h>
#include <KextHeaders/ObjectField.hpp>

class Backlight
{
    using t_DceDriverSetBacklight = void (*)(void *PanelControl, UInt32 backlightPwm);
    using t_DcLinkSetBacklightLevel = bool (*)(void *link, UInt32 backlightPwm, UInt32 frameRamp);
    using t_DcLinkSetBacklightLevelNits = bool (*)(void *link, bool IsHDR, UInt32 backlightMillinits,
        UInt32 TransitionTimeMs);

    bool ModuleLoaded {false};
        ObjectField<UInt8> DcLinkCapabillsField {};
    UInt32 CurrentPwmBacklightLevel {0}, MaxPwmBacklightLevel {0xFFFF};
    UInt32 MaxOLED {1000 * 512};
    IONotifier *DisplayNotif {nullptr};
    void *EmbeddedPanelLink {nullptr};
    bool SupportsAUX {false};
    t_DceDriverSetBacklight OrgDceDriverSetBacklight {nullptr};
    mach_vm_address_t OrgDcePanelCntlHwInit {0};
    void *PanelControlPtr {nullptr};
    mach_vm_address_t OrgLinkCreate {0};
    t_DcLinkSetBacklightLevel OrgDcLinkSetBacklightLevel {0};
    t_DcLinkSetBacklightLevelNits OrgDcLinkSetBacklightLevelNits {0};
    mach_vm_address_t OrgSetAttributeForConnection {0};
    mach_vm_address_t OrgGetAttributeForConnection {0};
    mach_vm_address_t OrgApplePanelSetDisplay {0};

    public:
        static Backlight &Singleton();

        void StartModule();

    private:
        void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size);

        static bool OnAppleBacklightDisplayLoad(void *Target, void *RefCon, IOService *NewService, IONotifier *Notifier);
        void RegisterDispMaxBrightnessNotif();

        static UInt32 WrapDcePanelCntlHwInit(void *PanelControl);
        static void *WrapLinkCreate(void *Data);
        static IOReturn WrapSetAttributeForConnection(IOService *Framebuffer, IOIndex ConnectIndex, IOSelect Attribute,
            uintptr_t Value);
        static IOReturn WrapGetAttributeForConnection(IOService *Framebuffer, IOIndex ConnectIndex, IOSelect Attribute,
            uintptr_t *Value);
        static size_t Return0();
        static bool WrapApplePanelSetDisplay(IOService *ThatIGuess, IODisplay *Display);
};