// NootedDeck
// 🛈 brurmonemt 2025
// © ChefKiss 2024-25
// 
// File: NootedDeck/DebugEnabler.cpp
// what is even the point of this description, i mean these file names explain themselves geez

#include <Headers/kern_api.hpp>
#include <KextHeaders/DebugEnabler.hpp>
#include <KextHeaders/NDeck.hpp>
#include <KextHeaders/Patcher+.hpp>

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

// Patterns

// X6000FB
static const UInt8 KDmLoggerWritePattern[] = {0x55, 0x48, 0x89, 0xE5, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54,
    0x53, 0x48, 0x81, 0xEC, 0x88, 0x04, 0x00, 0x00};

static const UInt8 KDalDmLoggerShouldLogPartialPattern[] = {0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x04, 0x81,
    0x0F, 0xA3, 0xD0, 0x0F, 0x92, 0xC0};
static const UInt8 KDalDmLoggerShouldLogPartialPatternMask[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Patches

// X6000FB: Enable all Display Core logs.
static const UInt8 KInitPopulateDcInitDataOriginal[] = {0x48, 0xB9, 0xDB, 0x1B, 0xFF, 0x7E, 0x10, 0x00, 0x00, 0x00};
static const UInt8 KInitPopulateDcInitDataPatched[] = {0x48, 0xB9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Enable more Display Core logs on Catalina (not sure how to enable all of them yet and also commented out because of the unknown Catalina support).
/* static const UInt8 KInitPopulateDcInitDataCatalinaOriginal[] = {0x48, 0xC7, 0x87, 0x20, 0x02, 0x00, 0x00, 0xDB, 0x1B,
    0xFF, 0x7E};
static const UInt8 KInitPopulateDcInitDataCatalinaPatched[] = {0x48, 0xC7, 0x87, 0x20, 0x02, 0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF}; */

// Enable all AmdBiosParserHelper logs.
static const UInt8 KBIOSParserHelperInitWithDataOriginal[] = {0x08, 0xC7, 0x07, 0x01, 0x00, 0x00, 0x00};
static const UInt8 KBIOSParserHelperInitWithDataPatched[] = {0x08, 0xC7, 0x07, 0xFF, 0x00, 0x00, 0x00};

// HWLibs: Enable all MCIL debug prints (debugLevel = 0xFFFFFFFF, mostly for PP_Log).
static const UInt8 kAtiPowerPlayServicesConstructorOriginal[] = {0x8B, 0x40, 0x60, 0x48, 0x8D};
static const UInt8 kAtiPowerPlayServicesConstructorPatched[] = {0x83, 0xC8, 0xFF, 0x48, 0x8D};

// Enable printing of all PSP event logs.
static const UInt8 KAMDLogPspOriginal[] = {0x83, 0x00, 0x02, 0x0F, 0x85, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x83, 0x00, 0x02, 0x72, 0x00, 0x41, 0x00, 0x00, 0x09, 0x02, 0x18, 0x00, 0x74, 0x00, 0x41, 0x00,
    0x00, 0x01, 0x06, 0x10, 0x00, 0x0f, 0x85, 0x00, 0x00, 0x00, 0x00};
static const UInt8 KAMDLogPspOriginalMask[] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
    0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
static const UInt8 KAMDLogPspPatched[] = {0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66,
    0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90,
    0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x66, 0x90, 0x90};

// Module logic

static DebugEnabler Instance {};

DebugEnabler &DebugEnabler::Singleton() { return Instance; }

enum GPUChannelDebugPolicy {
    CHANNEL_WAIT_FOR_PM4_IDLE = 0x1,
    CHANNEL_WAIT_FOR_TS_AFTER_SUBMISSION = 0x2,
    // 0x8, 0x10 = ??, PM4-related
    CHANNEL_DISABLE_PREEMPTION = 0x20,
};

enum GPUDebugPolicy {
    WAIT_FOR_PM4_IDLE = 0x1,
    WAIT_FOR_TS_AFTER_SUBMISSION = 0x2,
    PANIC_AFTER_DUMPING_LOG = 0x4,
    PANIC_ON_POWEROFF_REGISTER_ACCESS = 0x8,
    PRINT_FUNC_ENTRY_EXIT = 0x40,
    DBX_SLEEP_BEFORE_GPU_RESTART = 0x200,
    DISABLE_PREEMPTION = 0x80000000,
};

void DebugEnabler::StartModule()
{
    PANIC_COND(this->ModuleLoaded, "DebugEnabler", "Attempted to load module twice!");
    this->ModuleLoaded = true;

    SYSLOG("DebugEnabler", "Module loaded successfully!");

    if (!ADDPR(DebugEnabled)) { return; }

    lilu.onKextLoadForce(&RadeonX6000FramebufferKext);

    lilu.onKextLoadForce(
        nullptr, 0,
        [](void *user, KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
        {
            static_cast<DebugEnabler *>(user)->ProcessKext(Patcher, ID, Slide, Size);
        },
    this);
}

void DebugEnabler::ProcessX6000FB(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    NDeck::Singleton().SetProperty32Bit("PP_LogLevel", 0xFFFF);
    NDeck::Singleton().SetProperty32Bit("PP_LogSource", 0xFFFFFFFF);
    NDeck::Singleton().SetProperty32Bit("PP_LogDestination", 0xFFFFFFFF);
    NDeck::Singleton().SetProperty32Bit("PP_LogField", 0xFFFFFFFF);
    NDeck::Singleton().SetProperty32Bit("PP_DumpRegister", TRUE);
    NDeck::Singleton().SetProperty32Bit("PP_DumpSMCTable", TRUE);
    NDeck::Singleton().SetProperty32Bit("PP_LogDumpTableBuffers", TRUE);

    Patcher+::PatternRouteRequest Requests[] =
    {
        {"__ZN24AMDRadeonX6000_AmdLogger15initWithPciInfoEP11IOPCIDevice", WrapSomethingWithPCIInfo,
            this->OrgInitWithPCIInfo},
        {"__ZN34AMDRadeonX6000_AmdRadeonController10doGPUPanicEPKcz", DoGPUPanic},
        {"_dm_logger_write", DMLoggerWrite, kDmLoggerWritePattern},
    };
    PANIC_COND(!Patcher+::PatternRouteRequest::RouteAll(Patcher, ID, Requests, Slide, Size), "DebugEnabler",
        "Failed to route X6000FB debug symbols");

    // Enable all DalDmLogger logs
    // TODO: Maybe replace this with some simpler patches?
    auto *LogEnableMaskMinors =
        Patcher.solveSymbol<void *>(ID, "__ZN14AmdDalDmLogger19LogEnableMaskMinorsE", Slide, Size);
    Patcher.clearError();
    if (LogEnableMaskMinors == nullptr)
    {
        size_t Offset = 0;
        PANIC_COND(!KernelPatcher::findPattern(kDalDmLoggerShouldLogPartialPattern,
                       kDalDmLoggerShouldLogPartialPatternMask, arrsize(kDalDmLoggerShouldLogPartialPattern),
                       reinterpret_cast<const void *>(Slide), Size, &Offset),
            "DebugEnabler", "Failed to solve LogEnableMaskMinors");
        auto *instAddr = reinterpret_cast<UInt8 *>(Slide + Offset);
        // inst + instSize + imm32 = addr
        LogEnableMaskMinors = instAddr + 7 + *reinterpret_cast<SInt32 *>(instAddr + 3);
    }
    PANIC_COND(MachInfo::setKernelWriting(true, KernelPatcher::kernelWriteLock) != KERN_SUCCESS, "DebugEnabler",
        "Failed to enable kernel writing");
    memset(LogEnableMaskMinors, 0xFF, 0x80);
    MachInfo::setKernelWriting(false, KernelPatcher::kernelWriteLock);

    // Enable all Display Core logs... soon
    /* if (NDeck::Singleton().GetAttributes().IsCatalinaB()) {
        const Patcher+::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kInitPopulateDcInitDataCatalinaOriginal,
            kInitPopulateDcInitDataCatalinaPatched, 1};
        PANIC_COND(!Patch.apply(Patcher, Slide, Size), "DebugEnabler",
            "Failed to apply populateDcInitData Patch (10.15)");
    } else { */
        const Patcher+::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kInitPopulateDcInitDataOriginal,
            kInitPopulateDcInitDataPatched, 1};
        PANIC_COND(!Patch.apply(Patcher, Slide, Size), "DebugEnabler", "Failed to apply populateDcInitData patch");
    // }

    // Enable all bios parser logs
    const Patcher+::MaskedLookupPatch Patch {&RadeonX6000FramebufferKext, kBiosParserHelperInitWithDataOriginal,
        kBiosParserHelperInitWithDataPatched, 1};
    PANIC_COND(!Patch.apply(Patcher, Slide, Size), "DebugEnabler",
        "Failed to apply AmdBiosParserHelper::initWithData patch");
}

void DebugEnabler::ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size)
{
    if (RadeonX6000FramebufferKext.loadIndex == ID)
    {
        this->ProcessX6000FB(Patcher, ID, Slide, Size);
    }
}

bool DebugEnabler::WrapSomethingWithPCIInfo(void *ThatIGuess, void *AbsolutelyAPCIDevice)
{
    auto ReturnValue = FunctionCast(WrapSomethingWithPCIInfo, Singleton().OrgInitWithPCIInfo)(ThatIGuess, AbsolutelyAPCIDevice);
    getMember<UInt64>(ThatIGuess, 0x28) = 0xFFFFFFFFFFFFFFFF;    // Enable all log types
    getMember<UInt32>(ThatIGuess, 0x30) = 0xFF;                  // Enable all log severities
    return ReturnValue;
}

void DebugEnabler::DoGPUPanic(void *, char const *FMT, ...)
{
    va_list VA;
    va_start(VA, FMT);
    auto *buf = static_cast<char *>(IOMalloc(1000));
    bzero(buf, 1000);
    vsnprintf(buf, 1000, FMT, VA);
    va_end(VA);

    DBGLOG("DebugEnabler", "DoGPUPanic: %s", buf);
    IOSleep(10000);
    panic("%s", buf);
}

static const char *LogTypes[] = {"Error", "Warning", "Debug", "DC_Interface", "DTN", "Surface", "HW_Hotplug", "HW_LKTN",
    "HW_Mode", "HW_Resume", "HW_Audio", "HW_HPDIRQ", "MST", "Scaler", "BIOS", "BWCalcs", "BWValidation", "I2C_AUX",
    "Sync", "Backlight", "Override", "Edid", "DP_Caps", "Resource", "DML", "Mode", "Detect", "LKTN", "LinkLoss",
    "Underflow", "InterfaceTrace", "PerfTrace", "DisplayStats"};

// Needed to prevent stack overflow
void DebugEnabler::DMLoggerWrite(void *, const UInt32 LogType, const char *FMT, ...)
{
    va_list VA;
    va_start(VA, FMT);
    auto *Message = static_cast<char *>(IOMalloc(0x1000));
    vsnprintf(Message, 0x1000, FMT, VA);
    va_end(VA);
    auto *Epilogue = Message[strnlen(Message, 0x1000) - 1] == '\n' ? "" : "\n";
    if (LogType < arrsize(LogTypes))
    {
        kprintf("[%s]\t%s%s", LogTypes[LogType], Message, Epilogue);
    }
    else
    {
        kprintf("%s%s", Message, Epilogue);
    }
    IOFree(Message, 0x1000);
}

// Port of `AmdTtlServices::cosDebugAssert` for empty `_*_assertion` functions
// Get doxxed ner di have your pi address 192.125.123.456
void DebugEnabler::IPAssertion(void *, UInt32 Condition, const char *Function, const char *File, UInt32 Line, const char *Msg)
{
    if (Condition != 0) { return; }

    kprintf("AMD TTL COS: \n----------------------------------------------------------------\n");
    kprintf("AMD TTL COS: ASSERT FUNCTION: %s\n", safeString(Function));
    kprintf("AMD TTL COS: ASSERT FILE: %s\n", safeString(File));
    kprintf("AMD TTL COS: ASSERT LINE: %d\n", Line);
    kprintf("AMD TTL COS: ASSERT REASON: %s\n", safeString(Msg));
    kprintf("AMD TTL COS: \n----------------------------------------------------------------\n");
}

void DebugEnabler::GCDebugPrint(void *, const char *FMT, ...)
{
    kprintf("[GC DEBUG]: ");
    va_list args;
    va_start(args, FMT);
    vprintf(FMT, args);
    va_end(args);
}

void DebugEnabler::PlayStationPortableDebugPrint(void *, const char *FMT, ...) // interesting name there chefkiss
{
    kprintf("[PSP DEBUG]: ");
    va_list args;
    va_start(args, FMT);
    vprintf(FMT, args);
    va_end(args);
}

bool DebugEnabler::WrapGetNumericProperty(void *ThatIGuess, const char *Name, UInt32 *Value)
{
    auto ReturnValue = FunctionCast(WrapGetNumericProperty, Singleton().OrgGetNumericProperty)(ThatIGuess, Name, Value);
    if (Name == nullptr || strncmp(Name, "GpuDebugPolicy", 15) != 0) { return ReturnValue; }
    if (Value != nullptr)
    {
        // Enable entry traces
        if (ReturnValue)
        {
            *Value |= PRINT_FUNC_ENTRY_EXIT;
        }
        else
        {
            *Value = PRINT_FUNC_ENTRY_EXIT;
        }
    }
    return true;
}
