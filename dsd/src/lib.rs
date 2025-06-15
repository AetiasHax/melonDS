pub mod relocs;

use relocs::*;

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        type AmbiguousRelocation;

        fn dsd_get_ambiguous_relocations(config_path: &str) -> Result<Vec<AmbiguousRelocation>>;

        fn dsd_melonds_init() -> u32;
    }
}

fn dsd_melonds_init() -> u32 {
    42
}
