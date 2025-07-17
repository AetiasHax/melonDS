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
        /// The source autoload ID, or `-1` if not applicable.
        pub source_autoload: i16,
        /// Two or more target overlay IDs that this relocation can point to.
        pub target_overlays: Vec<u16>,
        /// The kind of relocation.
        pub kind: RelocationKind,
    }

    struct OverlayLoadFunctions {
        pub load: u32,
        pub unload: u32,
    }

    struct OverlayInfo {
        pub id: u16,
        pub start_address: u32,
        pub end_address: u32,
    }

    extern "Rust" {
        fn dsd_get_ambiguous_relocations(config_path: &str) -> Vec<AmbiguousRelocation>;
        fn dsd_disambiguate_relocation(
            config_path: &str,
            source_overlay: i16,
            source_autoload: i16,
            from: u32,
            target_overlay: u16,
        ) -> ();

        fn dsd_get_overlay_load_functions(config_path: &str) -> OverlayLoadFunctions;
        fn dsd_get_overlay_info(config_path: &str) -> Vec<OverlayInfo>;

        fn dsd_melonds_init() -> u32;
    }
}

pub use ffi::*;

fn dsd_melonds_init() -> u32 {
    42
}
