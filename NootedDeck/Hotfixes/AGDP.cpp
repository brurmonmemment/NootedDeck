// TODO: label this file

#include <Headers/kern_api.hpp>
#include <KextHeaders/Hotfixes/AGDP.hpp>
#include <KextHeaders/NDeck.hpp>
#include <KextHeaders/Patcher+.hpp>

static const char *AGDPPath = "/System/Library/Extensions/AppleGraphicsControl.kext/Contents/PlugIns/"
                              "AppleGraphicsDevicePolicy.kext/Contents/MacOS/AppleGraphicsDevicePolicy";
static KernelPatcher::KextInfo AGDPKext {
    "com.apple.driver.AppleGraphicsDevicePolicy",
    &AGDPPath,
    1,
    {true},
    {},
    KernelPatcher::KextInfo::Unloaded,
};

// Patches

// Change framebuffer count >= 2 check to >= 1.
static const UInt8 KAGDPFBCountCheckOriginal[] = {0x02, 0x00, 0x00, 0x83, 0xF8, 0x02};
static const UInt8 KAGDPFBCountCheckPatched[] = {0x02, 0x00, 0x00, 0x83, 0xF8, 0x01};

// Ditto
static const UInt8 KAGDPFBCountCheckOriginal13[] = {0x41, 0x83, 0xBE, 0x14, 0x02, 0x00, 0x00, 0x02};
static const UInt8 KAGDPFBCountCheckPatched13[] = {0x41, 0x83, 0xBE, 0x14, 0x02, 0x00, 0x00, 0x01};

// Neutralize access to AGDP configuration by board identifier.
static const UInt8 KAGDPBoardIDKeyOriginal[] = "board-ID";
static const UInt8 KAGDPBoardIDKeyPatched[] = "applehax";

// Module logic

static Hotfixes::AGDP Instance {};

Hotfixes::AGDP &Hotfixes::AGDP::Singleton() { return Instance; }

void Hotfixes::AGDP::StartModule()
{
    PANIC_COND(this->ModuleLoaded, "AGDP", "Attempted to load module twice!");
    this->ModuleLoaded = true;

    SYSLOG("AGDP", "Module loaded successfully!");

    lilu.onKextLoadForce(
        &AGDPKext, 1,
        [](void *user, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size) {
            static_cast<Hotfixes::AGDP *>(user)->ProcessKext(Patcher, ID, Slide, Size);
        },
        this);
}

void Hotfixes::AGDP::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    if (AGDPKext.loadIndex != ID) { return; }

    const Patcher+::MaskedLookupPatch boardIdPatch {&AGDPKext, KAGDPBoardIDKeyOriginal, KAGDPBoardIDKeyPatched, 1};
    SYSLOG_COND(!boardIdPatch.apply(Patcher, Slide, Size), "AGDP", "Failed to apply AGDP board-ID Patch");

    if (NDeck::Singleton().GetAttributes().IsVentura())
    {
        const Patcher+::MaskedLookupPatch Patch {&AGDPKext, KAGDPFBCountCheckOriginal13, KAGDPFBCountCheckPatched13,
            1};
        SYSLOG_COND(!Patch.apply(Patcher, Slide, Size), "AGDP", "Failed to apply AGDP FB count check Patch");
    }
    else
    {
        const Patcher+::MaskedLookupPatch Patch {&AGDPKext, KAGDPFBCountCheckOriginal, KAGDPFBCountCheckPatched, 1};
        SYSLOG_COND(!Patch.apply(Patcher, Slide, Size), "AGDP", "Failed to apply AGDP FB count check Patch");
    }
}