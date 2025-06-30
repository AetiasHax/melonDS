#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>

#include "dsd_melonds.h"

struct TrackedReloc
{
    const AmbiguousRelocation *reloc;
    int32_t offset;

    TrackedReloc() : reloc(nullptr), offset(0) {}
    TrackedReloc(const AmbiguousRelocation *reloc) : reloc(reloc), offset(0) {}
    TrackedReloc(const AmbiguousRelocation *reloc, int32_t offset) : reloc(reloc), offset(offset) {}

    void Clear()
    {
        reloc = nullptr;
        offset = 0;
    }

    uint32_t To() const
    {
        return reloc ? reloc->to + offset : 0;
    }
};

class RelocTracker
{
private:
    TrackedReloc registers[16];
    std::unordered_map<uint32_t, TrackedReloc> memory;

public:
    void TrackRegister(uint32_t reg, TrackedReloc reloc);
    void TrackMemory(uint32_t addr, TrackedReloc reloc);

    void ForgetRegister(uint32_t reg);
    void ForgetMemory(uint32_t addr);
    void ForgetRelocation(const AmbiguousRelocation *reloc);

    TrackedReloc *GetRegister(uint32_t reg);
    TrackedReloc *GetMemory(uint32_t addr);
};

class DSD
{
public:
    std::string configPath;

    rust::Vec<AmbiguousRelocation> ambiguousRelocations;
    std::unordered_map<uint32_t, std::vector<const AmbiguousRelocation *>> ambiguousRelocationMap;
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
    void FunctionCalled(uint32_t addr, uint32_t pc);
    void MemoryLoaded(uint32_t addr, uint32_t reg, uint32_t value);
    void MemoryStored(uint32_t addr, uint32_t reg, uint32_t value);
    void ProcessedData(uint32_t destReg, uint32_t srcReg, uint32_t value, int32_t offset);
    void ProcessedAdd(uint32_t destReg, uint32_t srcRegA, uint32_t valueA, uint32_t srcRegB, uint32_t valueB);

    void RemoveRelocation(const AmbiguousRelocation *reloc);
    const AmbiguousRelocation *FindRelocation(uint32_t from, uint32_t to);
    void DisambiguateRelocation(const AmbiguousRelocation *reloc);
};
