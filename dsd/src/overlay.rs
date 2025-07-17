use std::path::PathBuf;

use anyhow::{Context, Result};
use ds_decomp::config::{config::Config, delinks::Delinks, module::ModuleKind, symbol::SymbolMaps};

use crate::{OverlayInfo, OverlayLoadFunctions};

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

pub fn dsd_get_overlay_info(config_path: &str) -> Vec<OverlayInfo> {
    match get_overlay_info(config_path) {
        Ok(info) => info,
        Err(e) => {
            eprintln!("Failed to get overlay info: {e}");
            vec![]
        }
    }
}

fn get_overlay_info(config_path: &str) -> Result<Vec<OverlayInfo>> {
    let config_path = PathBuf::from(config_path);
    let config = Config::from_file(&config_path)?;
    let config_dir = config_path.parent().context("Config path must have a parent directory")?;

    let mut infos = config
        .overlays
        .iter()
        .map(|overlay| {
            let delinks = Delinks::from_file(config_dir.join(&overlay.module.delinks), ModuleKind::Overlay(overlay.id))?;
            let start_address = delinks.sections.base_address().context("Base address not found")?;
            let end_address = delinks.sections.end_address().context("End address not found")?;
            Ok(OverlayInfo { id: overlay.id, start_address, end_address })
        })
        .collect::<Result<Vec<_>>>()?;

    infos.sort_unstable_by_key(|info| info.id);

    Ok(infos)
}
