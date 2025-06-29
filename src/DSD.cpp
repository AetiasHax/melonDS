#include <utility>

#include "DSD.h"
#include "NDS.h"

void DSD::Init(const char *path)
{
    this->configPath = path;

    this->ambiguousRelocations = std::move(dsd_get_ambiguous_relocations(path));
    this->ambiguousRelocationMap.clear();
    for (const AmbiguousRelocation &reloc : this->ambiguousRelocations)
    {
        this->ambiguousRelocationMap[reloc.from].push_back(&reloc);
    }

    this->overlayLoadFunctions = dsd_get_overlay_load_functions(path);

    printf("Loaded %zu ambiguous relocations\n", this->ambiguousRelocations.size());

    // for (const AmbiguousRelocation &reloc : this->ambiguousRelocations)
    // {
    //     printf("From: %08x To: %08x Source: %d, Target: ",
    //            reloc.from, reloc.to, reloc.source_overlay);
    //     for (const uint16_t target : reloc.target_overlays)
    //     {
    //         printf("%d,", target);
    //     }
    //     printf("\n");
    // }

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

    auto reloc = relocTracker.GetRegister(reg);
    if (reloc == nullptr || reloc->reloc == nullptr)
    {
        return;
    }

    if (reloc->To() != value)
    {
        relocTracker.ForgetRegister(reg);
        return;
    }

    for (uint16_t targetOverlay : reloc->reloc->target_overlays)
    {
        if (this->loadedOverlays.find(targetOverlay) == this->loadedOverlays.end())
        {
            continue; // Skip if the target overlay is not loaded
        }

        const AmbiguousRelocation *relocPtr = reloc->reloc;

        relocTracker.ForgetRelocation(relocPtr);
        this->RemoveRelocation(relocPtr);
        dsd_disambiguate_relocation(this->configPath.c_str(), relocPtr->source_overlay, relocPtr->source_autoload,
                                    relocPtr->from, targetOverlay);

        printf("Disambiguated relocation from %08x to %08x, correct overlay is %d. %zu remaining\n",
               relocPtr->from, relocPtr->to, targetOverlay, this->ambiguousRelocationMap.size());
    }
}

void DSD::MemoryLoaded(uint32_t addr, uint32_t reg, uint32_t value)
{
    // printf("Memory loaded at %08x into register %d with value %08x\n", addr, reg, value);

    const auto &relocsIt = this->ambiguousRelocationMap.find(addr);
    if (relocsIt != this->ambiguousRelocationMap.end())
    {
        const std::vector<const AmbiguousRelocation *> &relocs = relocsIt->second;
        for (const AmbiguousRelocation *reloc : relocs)
        {
            if (reloc->source_overlay != -1 && this->loadedOverlays.find(reloc->source_overlay) == this->loadedOverlays.end())
            {
                continue; // Skip if the source overlay is not loaded
            }
            if (value != reloc->to)
            {
                continue; // Skip if the value does not match the relocation target
            }
            // printf("Ambiguous relocation loaded into r%d: %08x -> %08x (source overlay %d)\n", reg, reloc->from, reloc->to, reloc->source_overlay);

            relocTracker.TrackRegister(reg, TrackedReloc(reloc));
        }
        return;
    }

    auto reloc = relocTracker.GetMemory(addr);
    if (reloc == nullptr)
    {
        return;
    }

    if (reloc->To() != value)
    {
        relocTracker.ForgetMemory(addr);
        return;
    }

    relocTracker.TrackRegister(reg, *reloc);
}

void DSD::MemoryStored(uint32_t addr, uint32_t reg, uint32_t value)
{
    // printf("Memory stored at %08x from register %d\n", addr, reg);

    auto reloc = relocTracker.GetRegister(reg);
    if (reloc == nullptr || reloc->reloc == nullptr)
    {
        return;
    }

    if (reloc->To() != value)
    {
        relocTracker.ForgetRegister(reg);
        return;
    }

    relocTracker.TrackMemory(addr, *reloc);
}

void DSD::ProcessedData(uint32_t destReg, uint32_t srcReg, uint32_t value, int32_t offset)
{
    // printf("Processed data: destReg %d, srcReg %d, offset %d\n", destReg, srcReg, offset);

    relocTracker.ForgetRegister(destReg);

    auto srcReloc = relocTracker.GetRegister(srcReg);
    if (srcReloc == nullptr || srcReloc->reloc == nullptr)
    {
        return;
    }

    if (srcReloc->To() != value)
    {
        relocTracker.ForgetRegister(srcReg);
        return;
    }

    relocTracker.TrackRegister(destReg, TrackedReloc(srcReloc->reloc, offset + srcReloc->offset));
}

void DSD::ProcessedAdd(uint32_t destReg, uint32_t srcRegA, uint32_t valueA, uint32_t srcRegB, uint32_t valueB)
{
    // printf("Processed add: destReg %d, srcRegA %d (value %08x), srcRegB %d (value %08x)\n",
    //        destReg, srcRegA, valueA, srcRegB, valueB);

    relocTracker.ForgetRegister(destReg);

    auto relocA = relocTracker.GetRegister(srcRegA);
    auto relocB = relocTracker.GetRegister(srcRegB);

    if (relocA != nullptr && relocA->reloc != nullptr)
    {
        if (relocA->To() != valueA)
        {
            relocTracker.ForgetRegister(srcRegA);
            return;
        }
        relocTracker.TrackRegister(destReg, TrackedReloc(relocA->reloc, relocA->offset + valueB));
    }
    else if (relocB != nullptr && relocB->reloc != nullptr)
    {
        if (relocB->To() != valueB)
        {
            relocTracker.ForgetRegister(srcRegB);
            return;
        }
        relocTracker.TrackRegister(destReg, TrackedReloc(relocB->reloc, relocB->offset + valueA));
    }
}

void DSD::RemoveRelocation(const AmbiguousRelocation *reloc)
{
    auto it = this->ambiguousRelocationMap.find(reloc->from);
    if (it == this->ambiguousRelocationMap.end())
    {
        return;
    }
    // printf("Relocations found\n");

    auto &relocs = it->second;
    auto relocIt = std::find(relocs.begin(), relocs.end(), reloc);
    if (relocIt == relocs.end())
    {
        return;
    }
    // printf("Relocation found\n");

    relocs.erase(relocIt);
    if (relocs.empty())
    {
        this->ambiguousRelocationMap.erase(it);
        // printf("Entry removed\n");
    }
}

void RelocTracker::TrackRegister(uint32_t reg, TrackedReloc reloc)
{
    if (reg > 15)
    {
        printf("Invalid register number: %d\n", reg);
        return;
    }

    this->registers[reg] = reloc;
}

void RelocTracker::TrackMemory(uint32_t addr, TrackedReloc reloc)
{
    this->memory[addr] = reloc;
}

void RelocTracker::ForgetRegister(uint32_t reg)
{
    if (reg > 15)
    {
        printf("Invalid register number: %d\n", reg);
        return;
    }

    this->registers[reg].Clear();
}

void RelocTracker::ForgetMemory(uint32_t addr)
{
    auto it = this->memory.find(addr);
    if (it != this->memory.end())
    {
        this->memory.erase(it);
    }
}

void RelocTracker::ForgetRelocation(const AmbiguousRelocation *reloc)
{
    for (uint32_t reg = 0; reg < 16; ++reg)
    {
        if (this->registers[reg].reloc == reloc)
            this->registers[reg].Clear();
    }

    for (auto it = this->memory.begin(); it != this->memory.end();)
    {
        if (it->second.reloc == reloc)
            it = this->memory.erase(it);
        else
            ++it;
    }
}

TrackedReloc *RelocTracker::GetRegister(uint32_t reg)
{
    if (reg > 15)
    {
        printf("Invalid register number: %d\n", reg);
        return nullptr;
    }

    return &this->registers[reg];
}

TrackedReloc *RelocTracker::GetMemory(uint32_t addr)
{
    auto it = this->memory.find(addr);
    if (it != this->memory.end())
    {
        return &it->second;
    }
    return nullptr;
}

void melonDS::NDS::InitDsd(const char *configPath)
{
    dsd.Init(configPath);
}
