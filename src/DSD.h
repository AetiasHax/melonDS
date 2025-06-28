#pragma once

#include <unordered_set>

#include "dsd_melonds.h"

class DSD
{
public:
    rust::Vec<AmbiguousRelocation> ambiguousRelocations;
    OverlayLoadFunctions overlayLoadFunctions;

    std::unordered_set<uint32_t> loadedOverlays;

public:
    DSD(const char *configPath);

    void OverlayLoaded(uint32_t id);
    void OverlayUnloaded(uint32_t id);
    void PrintLoadedOverlays();
};
