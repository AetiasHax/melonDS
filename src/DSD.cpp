#include <utility>

#include "DSD.h"
#include "NDS.h"

DSD::DSD(const char *path) : ambiguousRelocations(std::move(dsd_get_ambiguous_relocations(path))),
                             overlayLoadFunctions(dsd_get_overlay_load_functions(path))
{
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
}

void melonDS::NDS::InitDsd(const char *configPath)
{
    dsd = new DSD(configPath);
}
