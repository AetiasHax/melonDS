use std::path::PathBuf;

use anyhow::{Context, Result};
use ds_decomp::config::{config::Config, symbol::SymbolMaps};

use crate::OverlayLoadFunctions;

pub fn dsd_get_overlay_load_functions(config_path: &str) -> OverlayLoadFunctions {
    match get_overlay_load_functions(config_path) {
        Ok(relocs) => relocs,
        Err(e) => {
            eprintln!("Failed to get overlay load functions: {e}");
            OverlayLoadFunctions { load: 0, unload: 0 }
        }
    }
}

fn get_overlay_load_functions(config_path: &str) -> Result<OverlayLoadFunctions> {
    let config_path = PathBuf::from(config_path);
    let config = Config::from_file(&config_path)?;
    let config_dir = config_path.parent().context("Config path must have a parent directory")?;

    let symbol_maps = SymbolMaps::from_config(config_dir, &config)?;
    let (_, _, load_overlay_fn) =
        symbol_maps.find_symbols_by_name("FS_LoadOverlay").next().context("FS_LoadOverlay function not found")?;
    let (_, _, unload_overlay_fn) =
        symbol_maps.find_symbols_by_name("FS_UnloadOverlay").next().context("FS_UnloadOverlay function not found")?;

    Ok(OverlayLoadFunctions { load: load_overlay_fn.addr, unload: unload_overlay_fn.addr })
}
