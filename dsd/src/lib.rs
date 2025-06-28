pub mod overlay;
pub mod relocs;
use overlay::*;
use relocs::*;

#[cxx::bridge]
mod ffi {
    enum RelocationKind {
        ArmCall,
        ThumbCall,
        ArmCallThumb,
        ThumbCallArm,
        ArmBranch,
        Load,
        OverlayId,
    }

    struct AmbiguousRelocation {
        /// The address of the relocation.
        pub from: u32,
        /// The address the relocation points to.
        pub to: u32,
        /// The source overlay ID, or `-1` if not applicable.
        pub source_overlay: i16,
        /// Two or more target overlay IDs that this relocation can point to.
        pub target_overlays: Vec<u16>,
        /// The kind of relocation.
        pub kind: RelocationKind,
    }

    struct OverlayLoadFunctions {
        pub load: u32,
        pub unload: u32,
    }

    extern "Rust" {
        fn dsd_get_ambiguous_relocations(config_path: &str) -> Result<Vec<AmbiguousRelocation>>;

        fn dsd_get_overlay_load_functions(config_path: &str) -> Result<OverlayLoadFunctions>;

        fn dsd_melonds_init() -> u32;
    }
}

pub use ffi::*;

fn dsd_melonds_init() -> u32 {
    42
}
