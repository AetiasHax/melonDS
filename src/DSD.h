#pragma once

#include <unordered_set>
#include <map>
#include <vector>
#include <string>

#include "dsd_melonds.h"

class RelocTracker
{
private:
    const AmbiguousRelocation *registers[16];
    std::map<uint32_t, const AmbiguousRelocation *> memory;

public:
    void TrackRegister(uint32_t reg, const AmbiguousRelocation *reloc);
    void TrackMemory(uint32_t addr, const AmbiguousRelocation *reloc);

    void ForgetRegister(uint32_t reg);
    void ForgetMemory(uint32_t addr);
    void ForgetRelocation(const AmbiguousRelocation *reloc);

    const AmbiguousRelocation *GetRegister(uint32_t reg) const;
    const AmbiguousRelocation *GetMemory(uint32_t addr) const;
};

class DSD
{
public:
    std::string configPath;

    rust::Vec<AmbiguousRelocation> ambiguousRelocations;
    std::map<uint32_t, std::vector<const AmbiguousRelocation *>> ambiguousRelocationMap;
    RelocTracker relocTracker;

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

    void RemoveRelocation(const AmbiguousRelocation *reloc);
};
