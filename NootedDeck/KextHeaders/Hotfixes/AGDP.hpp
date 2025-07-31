// TODO: label this file

#include <Headers/kern_patcher.hpp>

namespace Hotfixes {
    class AGDP {
        bool ModuleLoaded {false};

        public:
        static AGDP &Singleton();

        void StartModule();

        private:
        void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size);
    };
};    // namespace Hotfixes
