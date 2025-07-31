// NootedDeck
// 🛈 brurmonemt 2025
// 
// File: NootedDeck/KextHeaders/SKU.hpp
// header for the SKU class (NootedDeck/SKU.cpp)

#pragma once
#include <IOKit/pci/IOPCIDevice.h>

const char *GetSKUForDevice(IOPCIDevice *PCIDevice);