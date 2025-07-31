// NootedDeck
// 🛈 brurmonemt 2025
// © ChefKiss 2024-25
// 
// File: NootedDeck/KextHeaders/DebugEnabler.hpp
// goodbye descriptn

#include <Headers/kern_patcher.hpp>

class DebugEnabler {
    bool ModuleLoaded                       {false};
    mach_vm_address_t OrgInitWithPCIInfo    {0};
    mach_vm_address_t OrgGetNumericProperty {0};

    public:
        static DebugEnabler &Singleton();

        void StartModule();

    private:
        void ProcessX6000FB(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t size);
        void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t size);

        static bool WrapSomethingWithPCIInfo(void *ThatIGuess, void *AbsolutelyAPCIDevice);
        static void DoGPUPanic(void *ThatIGuess, char const *FMT, ...);
        static void DMLoggerWrite(void *logger, const UInt32 logType, const char *FMT, ...);
        static void IPAssertion(void *instance, UInt32 cond, const char *Function, const char *File, UInt32 Line,
            const char *Msg);
        static void GCDebugPrint(void *instance, const char *FMT, ...);
        static void PlayStationPortableDebugPrint(void *instance, const char *FMT, ...);
        static bool WrapGetNumericProperty(void *ThatIGuess, const char *name, UInt32 *value);
};