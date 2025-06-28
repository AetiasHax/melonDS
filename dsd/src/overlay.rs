use anyhow::Result;

use crate::OverlayLoadFunctions;

pub fn dsd_get_overlay_load_functions(config_path: &str) -> Result<OverlayLoadFunctions> {
    // TODO: Get functions from the config files

    Ok(OverlayLoadFunctions { load: 0x2042584, unload: 0x20425b4 })
}
