// TODO: label this file

#pragma once
#include <Headers/kern_patcher.hpp>
#include <Headers/kern_util.hpp>

namespace VanGogh {
    class AppleGFXHDA {
        bool ModuleLoaded {false};
        OSMetaClass *OrgFunctionGroupTahiti {nullptr};
        OSMetaClass *OrgWidget1002AAA0 {nullptr};
        mach_vm_address_t OrgCreateAppleHDAFunctionGroup {0};
        mach_vm_address_t OrgCreateAppleHDAWidget {0};

        public:
            static AppleGFXHDA &Singleton();

            void StartModule();
            void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size);

        private:
            static void *WrapCreateAppleHDAFunctionGroup(void *devId);
            static void *WrapCreateAppleHDAWidget(void *devId);
    };
};    // namespace VanGogh
