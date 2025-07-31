// NootedDeck
// 🛈 brurmonemt 2025
// 
// File: NootedDeck/Patcher+.cpp
// THE ULTIMATE KERNEL PATCHER...

#include <KextHeaders/Patcher+.hpp>

bool Patcher+::PatternSolveRequest::Solve(KernelPatcher &Patcher, const size_t ID, const mach_vm_address_t Start,
    const size_t Size)
{
    PANIC_COND(this->address == nullptr, "Patcher+", "this->address is null for sym `%s`", safeString(this->symbol));

    if (this->symbol != nullptr)
    {
        Patcher.clearError();
        if (Start == 0 || Size == 0)
        {
            *this->address = Patcher.solveSymbol(ID, this->symbol);
        }
        else
        {
            *this->address = Patcher.solveSymbol(ID, this->symbol, Start, Size, true);
        }
        if (*this->address != 0) { return true; }
        SYSLOG("Patcher+", "Failed to solve `%s` using symbol: %d", this->symbol, Patcher.getError());
    }

    if (this->pattern == nullptr || this->patternSize == 0)
    {
        PANIC_COND(this->symbol == nullptr, "Patcher+",
            "Improperly made pattern solve request; no symbol or pattern present");
        SYSLOG("Patcher+", "Cannot solve `%s` using pattern", this->symbol);
        return false;
    }

    DBGLOG("Patcher+", "Failed to solve `%s` using symbol: %d. Attempting to use pattern.", safeString(this->symbol),
        Patcher.getError());
    PANIC_COND(Start == 0 || Size == 0, "Patcher+", "Improper jump pattern route request; no start or size");

    size_t Offset = 0;
    if (!KernelPatcher::findPattern(this->pattern, this->mask, this->patternSize, reinterpret_cast<const void *>(Start),
            Size, &Offset))
    {
        SYSLOG("Patcher+", "Failed to solve `%s` using pattern", safeString(this->symbol));
        return false;
    }

    *this->address = Start + Offset;
    DBGLOG("Patcher+", "Resolved `%s` at 0x%llX", safeString(this->symbol), *this->address);
    return true;
}

bool Patcher+::PatternSolveRequest::SolveAll(KernelPatcher &Patcher, const size_t ID,
    PatternSolveRequest *const Requests, const size_t count, const mach_vm_address_t Start, const size_t Size)
{
    for (size_t i = 0; i < count; i++)
    {
        if (Requests[i].Solve(Patcher, ID, Start, Size))
        {
            DBGLOG("Patcher+", "Solved pattern request (i: %zu)", i);
        }
        else
        {
            DBGLOG("Patcher+", "Failed to solve pattern request (i: %zu)", i);
            return false;
        }
    }
    return true;
}

bool Patcher+::PatternRouteRequest::Route(KernelPatcher &Patcher, const size_t ID, const mach_vm_address_t Start,
    const size_t Size)
{
    if (this->symbol != nullptr)
    {
        Patcher.clearError();
        if (Start == 0 || Size == 0)
        {
            this->from = Patcher.solveSymbol(ID, this->symbol);
        }
        else
        {
            this->from = Patcher.solveSymbol(ID, this->symbol, Start, Size, true);
        }
    }

    if (this->from == 0)
    {
        if (this->pattern == nullptr || this->patternSize == 0)
        {
            PANIC_COND(this->symbol == nullptr, "Patcher+",
                "Improperly made pattern route request; no symbol or pattern present");
            SYSLOG("Patcher+", "Failed to route `%s` using symbol: %d", this->symbol, Patcher.getError());
            return false;
        }
        DBGLOG("Patcher+", "Failed to solve `%s` using symbol: %d. Attempting to use pattern.",
            safeString(this->symbol), Patcher.getError());
        PANIC_COND(Start == 0 || Size == 0, "Patcher+", "Improper pattern route request (`%s`); no start or size",
            safeString(this->symbol));
        size_t Offset = 0;
        if (!KernelPatcher::findPattern(this->pattern, this->mask, this->patternSize,
                reinterpret_cast<const void *>(Start), Size, &Offset))
        {
            SYSLOG("Patcher+", "Failed to route `%s` using pattern", safeString(this->symbol));
            return false;
        }
        this->from = Start + Offset;
        DBGLOG("Patcher+", "Resolved `%s` at 0x%llX", safeString(this->symbol), this->from);
    }

    // Workaround as Patcher internals will attempt to resolve
    // the symbol without checking if the `from` field is 0.
    this->symbol = nullptr;
    return Patcher.routeMultiple(ID, this, 1, Start, Size);
}

bool Patcher+::PatternRouteRequest::RouteAll(KernelPatcher &Patcher, const size_t ID,
    PatternRouteRequest *const Requests, const size_t count, const mach_vm_address_t Start, const size_t Size)
{
    for (size_t i = 0; i < count; i++)
    {
        if (Requests[i].Route(Patcher, ID, Start, Size))
        {
            DBGLOG("Patcher+", "Applied pattern route (i: %zu)", i);
        }
        else
        {
            DBGLOG("Patcher+", "Failed to apply pattern route (i: %zu)", i);
            return false;
        }
    }
    return true;
}

bool Patcher+::MaskedLookupPatch::Apply(KernelPatcher &Patcher, const mach_vm_address_t Start,
    const size_t Size) const
{
    if (this->findMask == nullptr && this->replaceMask == nullptr && this->skip == 0)
    {
        Patcher.clearError();
        Patcher.applyLookupPatch(this, reinterpret_cast<UInt8 *>(Start), Size);
        return Patcher.getError() == KernelPatcher::Error::NoError;
    }
    PANIC_COND(Start == 0 || Size == 0, "Patcher+", "Improper mask lookup patch; no start or size");
    return KernelPatcher::findAndReplaceWithMask(reinterpret_cast<UInt8 *>(Start), Size, this->find, this->Size,
        this->findMask, this->findMask ? this->size : 0, this->replace, this->Size, this->replaceMask,
        this->replaceMask ? this->size : 0, this->count, this->skip);
}

bool Patcher+::MaskedLookupPatch::ApplyAll(KernelPatcher &Patcher, const MaskedLookupPatch *const Patches,
    const size_t count, const mach_vm_address_t Start, const size_t Size, const bool Force)
{
    for (size_t i = 0; i < count; i++)
    {
        if (Patches[i].Apply(Patcher, Start, Size))
        {
            DBGLOG("Patcher+", "Applied patches[%zu]", i);
        }
        else
        {
            DBGLOG("Patcher+", "Failed to apply patches[%zu]", i);
            if (!Force) { return false; }
        }
    }
    return true;
}

mach_vm_address_t Patcher+::JumpInstDestination(const mach_vm_address_t Start, const mach_vm_address_t End)
{
    SInt64 Offset;

    if (Start == 0 || End == 0)
    {
        SYSLOG("Patcher+", "JumpInstDestination start AND/OR end IS 0!!!");
        return 0;
    }

    const auto Instr = *reinterpret_cast<const UInt8 *>(Start);
    size_t InstrSize;
    if (Instr == 0x0F)
    {
        InstrSize = sizeof(UInt8) + sizeof(UInt8) + sizeof(SInt32);
        if (Start + InstrSize > End) { return 0; }

        const auto Instr1 = *reinterpret_cast<const UInt8 *>(Start + sizeof(UInt8));
        if (Instr1 >= 0x80 && Instr1 <= 0x8F)
        {
            Offset = *reinterpret_cast<const SInt32 *>(Start + sizeof(UInt8) + sizeof(UInt8));
        }
        else
        {
            return 0;
        }
    }
    else if (Instr == 0xE8 || Instr == 0xE9)
    {
        InstrSize = sizeof(UInt8) + sizeof(SInt32);
        if (Start + InstrSize > End) { return 0; }

        Offset = *reinterpret_cast<const SInt32 *>(Start + sizeof(UInt8));
    }
    else if (Instr >= 0x70 && Instr <= 0x7F)
    {
        InstrSize = sizeof(UInt8) + sizeof(UInt8);
        if (Start + InstrSize > End) { return 0; }

        Offset = *reinterpret_cast<const SInt8 *>(Start + sizeof(UInt8));
    }
    else
    {
        return 0;
    }

    mach_vm_address_t Result;
    if (Offset < 0)
    {
        const auto AbsoluteOffset = static_cast<UInt64>(-Offset);
        if (AbsoluteOffset > Start)
        {   // This should never EVER happen!!!
            return 0;
        }
        Result = Start - AbsoluteOffset + InstrSize;
    }
    else
    {
        Result = Start + static_cast<UInt64>(Offset) + InstrSize;
    }

    return Result >= End ? 0 : Result;
}

bool Patcher+::JumpPatternRouteRequest::Route(KernelPatcher &Patcher, const size_t ID, const mach_vm_address_t Start,
    const size_t Size)
{
    if (this->symbol != nullptr)
    {
        Patcher.clearError();
        if (Start == 0 || Size == 0)
        {
            this->from = Patcher.solveSymbol(ID, this->symbol);
        }
        else
        {
            this->from = Patcher.solveSymbol(ID, this->symbol, Start, Size, true);
        }
    }

    if (this->from == 0)
    {
        if (this->pattern == nullptr || this->patternSize == 0)
        {
            PANIC_COND(this->symbol == nullptr, "Patcher+",
                "Improperly made jump pattern route request; no symbol or pattern present");
            DBGLOG("Patcher+", "Failed to solve `%s` using symbol: %d", safeString(this->symbol), Patcher.getError());
            return false;
        }
        DBGLOG("Patcher+", "Failed to solve `%s` using symbol: %d. Attempting to use jump pattern.",
            safeString(this->symbol), Patcher.getError());
        PANIC_COND(Start == 0 || Size == 0, "Patcher+", "Improper jump pattern route request (`%s`); no start or size",
            safeString(this->symbol));
        size_t Offset = 0;
        if (!KernelPatcher::findPattern(this->pattern, this->mask, this->patternSize,
                reinterpret_cast<const void *>(Start), Size, &Offset))
        {
            SYSLOG("Patcher+", "Failed to solve `%s` using jump pattern", safeString(this->symbol));
            return false;
        }
        this->from = JumpInstDestination(Start + Offset + this->jumpInstOff, Start + Size);
        if (this->from == 0)
        {
            SYSLOG("Patcher+", "Failed to solve `%s` using jump pattern", safeString(this->symbol));
            return false;
        }
        DBGLOG("Patcher+", "Resolved `%s` at 0x%llX", safeString(this->symbol), this->from);
    }

    auto HasOriginal = this->org != nullptr;
    auto Wrapper = Patcher.routeFunction(this->from, this->to, HasOriginal);
    if (HasOriginal)
    {
        if (Wrapper == 0) { return false; }
        *this->org = Wrapper;
        return true;
    }
    else
    {
        return Wrapper == 0;
    }
}

bool Patcher+::JumpPatternRouteRequest::RouteAll(KernelPatcher &Patcher, const size_t ID,
    JumpPatternRouteRequest *const Requests, const size_t count, const mach_vm_address_t Start, const size_t Size)
{
    for (size_t i = 0; i < count; i++)
    {
        if (Requests[i].Route(Patcher, ID, Start, Size))
        {
            DBGLOG("Patcher+", "Applied jump pattern route (i: %zu)", i);
        }
        else
        {
            DBGLOG("Patcher+", "Failed to apply jump pattern route (i: %zu)", i);
            return false;
        }
    }
    return true;
}