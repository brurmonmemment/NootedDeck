// NootedDeck
// 🛈 brurmonemt 2025
// 
// File: NootedDeck/Patcher+.hpp
// THE ULTIMATE KERNEL PATCHER... header! :)

#pragma once
#include <Headers/kern_patcher.hpp>

namespace Patcher+
{
    struct PatternSolveRequest : KernelPatcher::SolveRequest
    {
        const UInt8 *const Pattern {nullptr}, *const Mask {nullptr};
        const size_t patternSize {0};

        template<typename T>
        PatternSolveRequest(const char *s, T &addr) : KernelPatcher::SolveRequest {s, addr} {}

        template<typename T, typename P, const size_t N>
        PatternSolveRequest(const char *s, T &addr, const P (&Pattern)[N])
            : KernelPatcher::SolveRequest {s, addr}, Pattern {Pattern}, patternSize {N} {}

        template<typename T, typename P, const size_t N>
        PatternSolveRequest(const char *s, T &addr, const P (&Pattern)[N], const UInt8 (&Mask)[N])
            : KernelPatcher::SolveRequest {s, addr}, Pattern {Pattern}, Mask {Mask}, patternSize {N} {}

        bool Solve(KernelPatcher &Patcher, const size_t ID, const mach_vm_address_t Start, const size_t Size);

        static bool SolveAll(KernelPatcher &Patcher, const size_t ID, PatternSolveRequest *const Requests,
            const size_t count, const mach_vm_address_t Start, const size_t Size);

        template<size_t N>
        static bool SolveAll(KernelPatcher &Patcher, const size_t ID, PatternSolveRequest (&Requests)[N],
            const mach_vm_address_t Start, const size_t Size)
        {
            return SolveAll(Patcher, ID, Requests, N, Start, Size);
        }
    };

    struct PatternRouteRequest : KernelPatcher::RouteRequest
    {
        const UInt8 *const Pattern {nullptr}, *const Mask {nullptr};
        const size_t patternSize {0};

        template<typename T>
        PatternRouteRequest(const char *s, T t, mach_vm_address_t &o) : KernelPatcher::RouteRequest {s, t, o} {}

        template<typename T, typename O>
        PatternRouteRequest(const char *s, T t, O &o) : KernelPatcher::RouteRequest {s, t, o} {}

        template<typename T>
        PatternRouteRequest(const char *s, T t) : KernelPatcher::RouteRequest {s, t} {}

        template<typename T, typename P, const size_t N>
        PatternRouteRequest(const char *s, T t, mach_vm_address_t &o, const P (&Pattern)[N])
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, patternSize {N} {}

        template<typename T, typename O, typename P, const size_t N>
        PatternRouteRequest(const char *s, T t, O &o, const P (&Pattern)[N])
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, patternSize {N} {}

        template<typename T, typename P, const size_t N>
        PatternRouteRequest(const char *s, T t, const P (&Pattern)[N])
            : KernelPatcher::RouteRequest {s, t}, Pattern {Pattern}, patternSize {N} {}

        template<typename T, typename P, const size_t N>
        PatternRouteRequest(const char *s, T t, mach_vm_address_t &o, const P (&Pattern)[N], const UInt8 (&Mask)[N])
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, Mask {Mask}, patternSize {N} {}

        template<typename T, typename O, typename P, const size_t N>
        PatternRouteRequest(const char *s, T t, O &o, const P (&Pattern)[N], const UInt8 (&Mask)[N])
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, Mask {Mask}, patternSize {N} {}

        template<typename T, typename P, const size_t N>
        PatternRouteRequest(const char *s, T t, const P (&Pattern)[N], const UInt8 (&Mask)[N])
            : KernelPatcher::RouteRequest {s, t}, Pattern {Pattern}, Mask {Mask}, patternSize {N} {}

        bool Route(KernelPatcher &Patcher, const size_t ID, const mach_vm_address_t Start, const size_t Size);

        static bool RouteAll(KernelPatcher &Patcher, const size_t ID, PatternRouteRequest *const Requests, size_t count,
            const mach_vm_address_t Start, const size_t Size);

        template<size_t N>
        static bool RouteAll(KernelPatcher &Patcher, const size_t ID, PatternRouteRequest (&Requests)[N],
            const mach_vm_address_t Start, const size_t Size)
        {
            return RouteAll(Patcher, ID, Requests, N, Start, Size);
        }
    };

    struct MaskedLookupPatch : KernelPatcher::LookupPatch
    {
        const UInt8 *const findMask {nullptr}, *const replaceMask {nullptr};
        const size_t skip {0};

        MaskedLookupPatch(KernelPatcher::KextInfo *kext, const UInt8 *find, const UInt8 *replace, size_t Size,
            const size_t count, const size_t skip = 0)
            : KernelPatcher::LookupPatch {kext, find, replace, Size, count}, skip {skip} {}

        MaskedLookupPatch(KernelPatcher::KextInfo *kext, const UInt8 *find, const UInt8 *findMask, const UInt8 *replace,
            const size_t Size, const size_t count, const size_t skip = 0)
            : KernelPatcher::LookupPatch {kext, find, replace, Size, count}, findMask {findMask}, skip {skip} {}

        MaskedLookupPatch(KernelPatcher::KextInfo *kext, const UInt8 *find, const UInt8 *findMask, const UInt8 *replace,
            const UInt8 *replaceMask, const size_t Size, const size_t count, const size_t skip = 0)
            : KernelPatcher::LookupPatch {kext, find, replace, Size, count}, findMask {findMask},
              replaceMask {replaceMask}, skip {skip} {}

        template<const size_t N>
        MaskedLookupPatch(KernelPatcher::KextInfo *kext, const UInt8 (&find)[N], const UInt8 (&replace)[N],
            const size_t count, const size_t skip = 0)
            : MaskedLookupPatch {kext, find, replace, N, count, skip} {}

        template<const size_t N>
        MaskedLookupPatch(KernelPatcher::KextInfo *kext, const UInt8 (&find)[N], const UInt8 (&findMask)[N],
            const UInt8 (&replace)[N], const size_t count, const size_t skip = 0)
            : MaskedLookupPatch {kext, find, findMask, replace, N, count, skip} {}

        template<const size_t N>
        MaskedLookupPatch(KernelPatcher::KextInfo *kext, const UInt8 (&find)[N], const UInt8 (&findMask)[N],
            const UInt8 (&replace)[N], const UInt8 (&replaceMask)[N], const size_t count, const size_t skip = 0)
            : MaskedLookupPatch {kext, find, findMask, replace, replaceMask, N, count, skip} {}

        bool Apply(KernelPatcher &Patcher, const mach_vm_address_t Start, const size_t Size) const;

        static bool ApplyAll(KernelPatcher &Patcher, const MaskedLookupPatch *const Patches, const size_t count,
            const mach_vm_address_t Start, const size_t Size, const bool Force = false);

        template<const size_t N>
        static bool ApplyAll(KernelPatcher &Patcher, const MaskedLookupPatch (&Patches)[N],
            const mach_vm_address_t Start, const size_t Size, const bool Force = false)
        {
            return ApplyAll(Patcher, Patches, N, Start, Size, Force);
        }
    };

    mach_vm_address_t jumpInstDestination(const mach_vm_address_t Start, const mach_vm_address_t End);

    struct JumpPatternRouteRequest : KernelPatcher::RouteRequest
    {
        const UInt8 *const Pattern {nullptr}, *const Mask {nullptr};
        const size_t patternSize {0};
        const size_t jumpInstOff {0};

        template<typename T>
        JumpPatternRouteRequest(const char *s, T t) : KernelPatcher::RouteRequest {s, t} {}

        template<typename T>
        JumpPatternRouteRequest(const char *s, T t, mach_vm_address_t &o) : KernelPatcher::RouteRequest {s, t, o} {}

        template<typename T, typename O>
        JumpPatternRouteRequest(const char *s, T t, O &o) : KernelPatcher::RouteRequest {s, t, o} {}

        template<typename T, typename P, const size_t N>
        JumpPatternRouteRequest(const char *s, T t, mach_vm_address_t &o, const P (&Pattern)[N],
            const size_t jumpInstOff)
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, patternSize {N}, jumpInstOff {jumpInstOff} {}

        template<typename T, typename O, typename P, const size_t N>
        JumpPatternRouteRequest(const char *s, T t, O &o, const P (&Pattern)[N], const size_t jumpInstOff)
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, patternSize {N}, jumpInstOff {jumpInstOff} {}

        template<typename T, typename P, const size_t N>
        JumpPatternRouteRequest(const char *s, T t, const P (&Pattern)[N], const size_t jumpInstOff)
            : KernelPatcher::RouteRequest {s, t}, Pattern {Pattern}, patternSize {N}, jumpInstOff {jumpInstOff} {}

        template<typename T, typename P, const size_t N>
        JumpPatternRouteRequest(const char *s, T t, mach_vm_address_t &o, const P (&Pattern)[N], const UInt8 (&Mask)[N],
            const size_t jumpInstOff)
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, Mask {Mask}, patternSize {N},
              jumpInstOff {jumpInstOff} {}

        template<typename T, typename O, typename P, const size_t N>
        JumpPatternRouteRequest(const char *s, T t, O &o, const P (&Pattern)[N], const UInt8 (&Mask)[N],
            const size_t jumpInstOff)
            : KernelPatcher::RouteRequest {s, t, o}, Pattern {Pattern}, Mask {Mask}, patternSize {N},
              jumpInstOff {jumpInstOff} {}

        template<typename T, typename P, const size_t N>
        JumpPatternRouteRequest(const char *s, T t, const P (&Pattern)[N], const UInt8 (&Mask)[N],
            const size_t jumpInstOff)
            : KernelPatcher::RouteRequest {s, t}, Pattern {Pattern}, Mask {Mask}, patternSize {N},
              jumpInstOff {jumpInstOff} {}

        bool Route(KernelPatcher &Patcher, const size_t ID, const mach_vm_address_t Start, const size_t Size);

        static bool RouteAll(KernelPatcher &Patcher, const size_t ID, JumpPatternRouteRequest *const Requests,
            const size_t count, const mach_vm_address_t Start, const size_t Size);

        template<size_t N>
        static bool RouteAll(KernelPatcher &Patcher, const size_t ID, JumpPatternRouteRequest (&Requests)[N],
            const mach_vm_address_t Start, const size_t Size)
        {
            return RouteAll(Patcher, ID, Requests, N, Start, Size);
        }
    };
}