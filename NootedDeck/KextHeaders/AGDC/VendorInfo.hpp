// TODO: label this file

#pragma once
#include <IOKit/IOTypes.h>

typedef enum AGDCVendorClass {
    kAGDCVendorClassReserved,
    kAGDCVendorClassIntegratedGPU,
    kAGDCVendorClassDiscreteGPU,
    kAGDCVendorClassOtherHW,
    kAGDCVendorClassOtherSW,
    kAGDCVendorClassAppleGPUPolicyManager,
    kAGDCVendorClassAppleGPUPowerManager,
    kAGDCVendorClassGPURoot,
    kAGDCVendorClassAppleGPUWrangler,
    kAGDCVendorClassAppleMuxControl,
} AGDCVendorClass_t;

typedef struct AGDCVendorInfo {
    union {
        struct {
            UInt16 Minor;
            UInt16 Major;
        };
        UInt32 Raw;
    } Version;
    char VendorString[32];
    UInt32 VendorID;
    AGDCVendorClass_t VendorClass;
} AGDCVendorInfo_t;
