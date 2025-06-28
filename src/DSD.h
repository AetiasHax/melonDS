#pragma once

#include "dsd_melonds.h"

class DSD
{
private:
    rust::Vec<AmbiguousRelocation> ambiguousRelocations;
    OverlayLoadFunctions overlayLoadFunctions;

public:
    DSD(const char *configPath);
};
