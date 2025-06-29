#include <utility>

#include "DSD.h"
#include "NDS.h"

void DSD::Init(const char *path)
{
    this->ambiguousRelocations = std::move(dsd_get_ambiguous_relocations(path));
    this->overlayLoadFunctions = dsd_get_overlay_load_functions(path);

    for (const AmbiguousRelocation &reloc : this->ambiguousRelocations)
    {
        printf("From: %08x To: %08x Source: %d, Target: ",
               reloc.from, reloc.to, reloc.source_overlay);
        for (const uint16_t target : reloc.target_overlays)
        {
            printf("%d,", target);
        }
        printf("\n");
    }

    this->initialized = true;
}

void DSD::OverlayLoaded(uint32_t id)
{
    this->loadedOverlays.insert(id);
    // PrintLoadedOverlays();
}

void DSD::OverlayUnloaded(uint32_t id)
{
    this->loadedOverlays.erase(id);
    // PrintLoadedOverlays();
}

void DSD::PrintLoadedOverlays()
{
    printf("Loaded overlays: ");
    for (const uint32_t id : this->loadedOverlays)
    {
        printf("%d ", id);
    }
    printf("\n");
}

void DSD::RegisterDereferenced(uint32_t reg, uint32_t value)
{
    // printf("Register %d dereferenced\n", reg);
}

void DSD::MemoryLoaded(uint32_t addr, uint32_t reg, uint32_t value)
{
    // printf("Memory loaded at %08x into register %d with value %08x\n", addr, reg, value);
}

void DSD::MemoryStored(uint32_t addr, uint32_t reg, uint32_t value)
{
    // printf("Memory stored at %08x from register %d\n", addr, reg);
}

void DSD::ProcessedData(uint32_t destReg, uint32_t srcReg, uint32_t value, int32_t offset)
{
    // printf("Processed data: destReg %d, srcReg %d, offset %d\n", destReg, srcReg, offset);
}

void DSD::ProcessedAdd(uint32_t destReg, uint32_t srcRegA, uint32_t valueA, uint32_t srcRegB, uint32_t valueB)
{
    // printf("Processed add: destReg %d, srcRegA %d (value %08x), srcRegB %d (value %08x)\n",
    //        destReg, srcRegA, valueA, srcRegB, valueB);
}

void melonDS::NDS::InitDsd(const char *configPath)
{
    dsd.Init(configPath);
}
