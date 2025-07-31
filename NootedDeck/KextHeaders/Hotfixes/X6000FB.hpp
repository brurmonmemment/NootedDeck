// TODO: label this file

#include <Headers/kern_patcher.hpp>

namespace Hotfixes {
    class X6000FB {
        static constexpr UInt32 IOFBRequestControllerEnabled = 0x1B;

        bool KextLoaded {false};
        mach_vm_address_t OrgDpReceiverPowerCtrl {0};
        IOReturn (*orgMessageAccelerator)(void *ThatIGuess, UInt32 RequestType, void *Arg2, void *Arg3, void *Arg4) {nullptr};
        mach_vm_address_t OrgControllerPowerUp {0};

        public:
        static X6000FB &Singleton();

        void StartModule();

        private:
        void ProcessKext(KernelPatcher &Patcher, size_t ID, mach_vm_address_t Slide, size_t Size);

        static void wrapDpReceiverPowerCtrl(void *Link, bool PowerOn);
        static UInt32 wrapControllerPowerUp(void *ThatIGuess);
    };
};    // namespace Hotfixes
