// NootedDeck
// 🛈 brurmonemt 2025
//  
// File: NootedDeck/KextHeaders/NDeck.hpp
// main class' attributes (NootedDeck/NDeck.cpp)

#pragma once
#include <Headers/kern_util.hpp>
#include <IOKit/IOTypes.h>

class NDeckAttributes
{
    // OS ver integers
    static constexpr UInt16 IsCatalina = (1U << 0);
    static constexpr UInt16 IsBigSurPlus = (1U << 1);
    static constexpr UInt16 IsMonterey = (1U << 2);
    static constexpr UInt16 IsMontereyPlus = (1U << 3);
    static constexpr UInt16 IsVentura = (1U << 4);
    static constexpr UInt16 IsVenturaPlus = (1U << 5);
    static constexpr UInt16 IsVentura1304Based = (1U << 6);
    static constexpr UInt16 IsVentura1304Plus = (1U << 7);
    static constexpr UInt16 IsSonoma1404Plus = (1U << 8);

    public:
        // OS ver bools
        inline bool IsCatalinaBool()     const { return (this->Value & IsCatalina) != 0; }
        inline bool IsBigSurPlus()       const { return (this->Value & IsBigSurPlus) != 0; }
        inline bool IsMonterey()         const { return (this->Value & IsMonterey) != 0; }
        inline bool IsMontereyPlus()     const { return (this->Value & IsMontereyPlus) != 0; }
        inline bool IsVentura()          const { return (this->Value & IsVentura) != 0; }
        inline bool IsVenturaPlus()      const { return (this->Value & IsVenturaPlus) != 0; }
        inline bool IsVentura1304Based() const { return (this->Value & IsVentura1304Based) != 0; }
        inline bool IsVentura1304Plus()  const { return (this->Value & IsVentura1304Plus) != 0; }
        inline bool IsSonoma1404Plus()   const { return (this->Value & IsSonoma1404Plus) != 0; }

        // OS ver setters
        inline void SetCatalina()         { this->Value |= IsCatalina; }
        inline void SetBigSurPlus()       { this->Value |= IsBigSurPlus; }
        inline void SetMonterey()         { this->Value |= IsMonterey; }
        inline void SetMontereyPlus()     { this->Value |= IsMontereyPlus; }
        inline void SetVentura()          { this->Value |= IsVentura; }
        inline void SetVenturaPlus()      { this->Value |= IsVenturaPlus; }
        inline void SetVentura1304Based() { this->Value |= IsVentura1304Based; }
        inline void SetVentura1304Plus()  { this->Value |= IsVentura1304Plus; }
        inline void SetSonoma1404Plus()   { this->Value |= IsSonoma1404Plus; }
}