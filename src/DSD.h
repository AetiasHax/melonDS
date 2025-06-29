#pragma once

#include <unordered_set>

#include "dsd_melonds.h"

class DSD
{
public:
    rust::Vec<AmbiguousRelocation> ambiguousRelocations;
    OverlayLoadFunctions overlayLoadFunctions;

    std::unordered_set<uint32_t> loadedOverlays;

    bool initialized;

public:
    void Init(const char *configPath);

    void OverlayLoaded(uint32_t id);
    void OverlayUnloaded(uint32_t id);
    void PrintLoadedOverlays();

    void RegisterDereferenced(uint32_t reg, uint32_t value);
    void MemoryLoaded(uint32_t addr, uint32_t reg, uint32_t value);
    void MemoryStored(uint32_t addr, uint32_t reg, uint32_t value);
    void ProcessedData(uint32_t destReg, uint32_t srcReg, uint32_t value, int32_t offset);
    void ProcessedAdd(uint32_t destReg, uint32_t srcRegA, uint32_t valueA, uint32_t srcRegB, uint32_t valueB);
};
