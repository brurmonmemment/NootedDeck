// NootedDeck
// 🛈 brurmonemt 2025
//  
// File: NootedDeck/SKU.cpp
// defines SKUs, only 0932 and 0405 exist since steam deck

#include <Headers/kern_iokit.hpp>
#include <Headers/kern_util.hpp>
#include <KextHeaders/SKU.hpp>

enum struct MatchType
{
    RevOnly,
    RevSubsys,
};

struct SKU
{
    MatchType MT           {MatchType::RevOnly};
    UInt16 Revision        {0};
    UInt16 SubsystemID     {0};
    UInt16 SubsystemVendor {0};
    const char *Number     {nullptr};
};

struct DevicePair
{
    UInt16 DeviceID;
    const SKU *SKUs;
    size_t count;
    const char *Fallback;
};

static const SKU Dev163F[] =
{
    {MatchType::RevOnly, 0xAE, 0x0000, 0x0000, "AMD Custom GPU 0405"},
    {MatchType::RevSubsys, 0xAE, 0x0123, 0x1002, "AMD Custom GPU 0405"},
};

static const SKU Dev1435[] =
{
    {MatchType::RevOnly, 0xAE, 0x0000, 0x0000, "AMD Custom GPU 0932"},
    {MatchType::RevSubsys, 0xAE, 0x0123, 0x1002, "AMD Custom GPU 0932"},
};

static const DevicePair Devices[] =
{
    {0x163F, Dev163F, sizeof(Dev163F) / sizeof(Dev163F), "AMD Custom GPU"}, // Van Gogh "Aerith" (LCD)
    {0x1435, Dev1435, sizeof(Dev1435) / sizeof(Dev1435), "AMD Custom GPU"}, // Van Gogh "Sephiroth" (OLED)
};

const char *GetSKUForDevice(IOPCIDevice *PCIDevice)
{
    auto DeviceID        = WIOKit::readPCIConfigValue(PCIDevice, WIOKit::kIOPCIConfigDeviceID);
    auto RevisionID      = WIOKit::readPCIConfigValue(PCIDevice, WIOKit::kIOPCIConfigRevisionID);
    auto SubsystemID     = WIOKit::readPCIConfigValue(PCIDevice, WIOKit::kIOPCIConfigSubSystemID);
    auto SubsystemVendor = WIOKit::readPCIConfigValue(PCIDevice, WIOKit::kIOPCIConfigSubSystemVendorID);
    
    for (auto &DevicePair : Devices)
    {
        if (DevicePair.DeviceID == DeviceID)
        {
            for (size_t i = 0; i < DevicePair.count; i++)
            {
                auto &SKU = DevicePair.SKUs[i];
                if (SKU.Revision != RevisionID ||
                    (SKU.MT == MatchType::RevSubsys &&
                        (SKU.SubsystemID != SubsystemID || SKU.SubsystemVendor != SubsystemVendor)))
                {
                    continue;
                }

                return SKU.Number;
            }

            return DevicePair.Fallback;
        }
    }

    return nullptr;
}