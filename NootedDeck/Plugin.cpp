#include <Headers/kern_api.hpp>
#include <Headers/plugin_start.hpp>
#include <KextHeaders/NDeck.hpp>

static const char *DebugBootArg = "-NDeckDebug";

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal | LiluAPI::AllowInstallerRecovery | LiluAPI::AllowSafeMode,
    nullptr,
    0,
    &DebugBootArg,
    1,
    nullptr,
    0,
    KernelVersion::BigSur,
    KernelVersion::Tahoe,
    []() { NDeck::Singleton().StartModule(); },
};