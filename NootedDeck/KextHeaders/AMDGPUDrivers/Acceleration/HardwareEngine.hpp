// Label

#pragma once

enum AMDHWEngineType {
    KAMDHWEngineTypePM4 = 0,
    KAMDHWEngineTypeSDMA0,
    KAMDHWEngineTypeSDMA1,
    KAMDHWEngineTypeSDMA2,
    KAMDHWEngineTypeSDMA3,
/*    KAMDHWEngineTypeUVD0,
    KAMDHWEngineTypeUVD1,
    KAMDHWEngineTypeVCE, */ // maybe old logic?
    KAMDHWEngineTypeVCN0,
    KAMDHWEngineTypeVCN1,
    KAMDHWEngineTypeSAMU,
    KAMDHWEngineTypeMax,
};