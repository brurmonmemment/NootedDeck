// NootedDeck
// 🛈 brurmonemt 2025
// 
// File: NootedDeck/KextHeaders/ObjectField.hpp
// object field NO WAY WHO WOULDVE THOUGHT

#pragma once
#include <Headers/kern_util.hpp>

template<typename T>
class ObjectField {
    static constexpr UInt32 InvalidOffset = 0xFFFFFFFF;

    UInt32 Offset;

    constexpr ObjectField(const UInt32 Offset) : Offset {Offset} {}

    public:
    constexpr ObjectField() : ObjectField(InvalidOffset) {}

    inline void operator=(const UInt32 Other)
    {
        PANIC_COND(this->Offset != InvalidOffset, "ObjField", "Offset reassigned");
        this->Offset = Other;
    }

    inline ObjectField<T> operator+(const UInt32 Value)
    {
        PANIC_COND(this->Offset == InvalidOffset, "ObjField", "Uninitialised");
        return ObjectField<T> {this->Offset + Value};
    }

    inline T &get(void *const Object)
    {
        PANIC_COND(Object == nullptr, "ObjField", "Object parameter is null");
        PANIC_COND(this->Offset == InvalidOffset, "ObjField", "Uninitialised");
        return getMember<T>(Object, this->Offset);
    }

    inline void set(void *const Object, const T Value)
    {
        PANIC_COND(Object == nullptr, "ObjField", "Object parameter is null");
        PANIC_COND(this->Offset == InvalidOffset, "ObjField", "Uninitialised");
        getMember<T>(Object, this->Offset) = Value;
    }
};
