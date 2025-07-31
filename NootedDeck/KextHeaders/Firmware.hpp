// TODO: label this file

#pragma once
#include <Headers/kern_util.hpp>

struct FWMetadata {
    const UInt8 *Data;
    const void *Extra;
    const UInt32 Length;
};

struct FWDescriptor {
    const char *Name;
    const FWMetadata Metadata;
};

extern const struct FWDescriptor Firmware[];
extern const size_t FirmwareCount;

inline const FWMetadata &GetFirmwareNamed(const char *Name) {
    for (size_t i = 0; i < FirmwareCount; i++) {
        if (strcmp(Firmware[i].Name, Name)) { continue; }

        return Firmware[i].Metadata;
    }
    PANIC("FW", "'%s' not found", Name);
}
