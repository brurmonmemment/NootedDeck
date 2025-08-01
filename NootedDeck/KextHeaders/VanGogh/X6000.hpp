// TODO: label this file

#pragma once
#include <Headers/kern_patcher.hpp>
#include <KextHeaders/ObjectField.hpp>

namespace VanGogh {
    class X6000 {
        bool ModuleLoaded                            {false};
        ObjectField<UInt32> RegionBaseField          {};
        mach_vm_address_t OrgAllocateAMDHWDisplay    {0};
        mach_vm_address_t OrgInitDCNRegistersOffsets {0};

        public:
            static X6000 &Singleton();

            void StartModule();

        private:
            void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size);

            static bool AccelStartX6000();
            static void WrapInitDCNRegistersOffsets(void *ThatIGuess);
    };
};    // namespace VanGogh
