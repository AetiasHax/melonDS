use std::path::{Path, PathBuf};

use anyhow::{Context, Result};
use ds_decomp::config::{
    config::{Config, ConfigModule},
    relocations::{RelocationModule, Relocations},
};

use crate::{AmbiguousRelocation, RelocationKind};

impl From<ds_decomp::config::relocations::RelocationKind> for RelocationKind {
    fn from(value: ds_decomp::config::relocations::RelocationKind) -> Self {
        match value {
            ds_decomp::config::relocations::RelocationKind::ArmCall => RelocationKind::ArmCall,
            ds_decomp::config::relocations::RelocationKind::ThumbCall => RelocationKind::ThumbCall,
            ds_decomp::config::relocations::RelocationKind::ArmCallThumb => RelocationKind::ArmCallThumb,
            ds_decomp::config::relocations::RelocationKind::ThumbCallArm => RelocationKind::ThumbCallArm,
            ds_decomp::config::relocations::RelocationKind::ArmBranch => RelocationKind::ArmBranch,
            ds_decomp::config::relocations::RelocationKind::Load => RelocationKind::Load,
            ds_decomp::config::relocations::RelocationKind::OverlayId => RelocationKind::OverlayId,
        }
    }
}

pub fn dsd_get_ambiguous_relocations(config_path: &str) -> Vec<AmbiguousRelocation> {
    match get_ambiguous_relocations(config_path) {
        Ok(relocs) => relocs,
        Err(e) => {
            eprintln!("Failed to get ambiguous relocations: {}", e);
            vec![]
        }
    }
}

fn get_ambiguous_relocations(config_path: &str) -> Result<Vec<AmbiguousRelocation>> {
    let config_path = PathBuf::from(config_path);
    let config_dir = config_path.parent().context("Config path must have a parent directory")?;
    let config = Config::from_file(&config_path)?;

    let main = AmbiguousRelocation::get_in_module(&config.main_module, config_dir, None, None)?;
    let autoloads = config
        .autoloads
        .iter()
        .enumerate()
        .map(|(index, autoload)| AmbiguousRelocation::get_in_module(&autoload.module, config_dir, None, Some(index as i16)))
        .collect::<Result<Vec<_>>>()?;
    let overlays = config
        .overlays
        .iter()
        .map(|overlay| AmbiguousRelocation::get_in_module(&overlay.module, config_dir, Some(overlay.id), None))
        .collect::<Result<Vec<_>>>()?;

    let mut ambiguous_relocs = main;
    for mut autoload in autoloads {
        ambiguous_relocs.append(&mut autoload);
    }
    for mut overlay in overlays {
        ambiguous_relocs.append(&mut overlay);
    }

    Ok(ambiguous_relocs)
}

impl AmbiguousRelocation {
    fn get_in_module(
        config: &ConfigModule,
        config_dir: &Path,
        source_overlay: Option<u16>,
        source_autoload: Option<i16>,
    ) -> Result<Vec<AmbiguousRelocation>> {
        let relocs = Relocations::from_file(config_dir.join(&config.relocations))?;
        let ambiguous_relocs = relocs
            .iter()
            .filter_map(|reloc| match reloc.module() {
                RelocationModule::Overlays { ids } => Some(AmbiguousRelocation {
                    from: reloc.from_address(),
                    to: reloc.to_address(),
                    source_overlay: source_overlay.map(|id| id as i16).unwrap_or(-1),
                    source_autoload: source_autoload.unwrap_or(-1),
                    target_overlays: ids.clone(),
                    kind: reloc.kind().into(),
                }),
                _ => None,
            })
            .collect::<Vec<_>>();

        Ok(ambiguous_relocs)
    }
}

pub fn dsd_disambiguate_relocation(
    config_path: &str,
    source_overlay: i16,
    source_autoload: i16,
    from: u32,
    target_overlay: u16,
) {
    if let Err(e) = disambiguate_relocation(config_path, source_overlay, source_autoload, from, target_overlay) {
        eprintln!("Failed to disambiguate relocation: {}", e);
    }
}

fn disambiguate_relocation(
    config_path: &str,
    source_overlay: i16,
    source_autoload: i16,
    from: u32,
    target_overlay: u16,
) -> Result<()> {
    let config_path = PathBuf::from(config_path);
    let config_dir = config_path.parent().context("Config path must have a parent directory")?;
    let config = Config::from_file(&config_path)?;

    let relocs_path = if source_overlay >= 0 {
        config_dir.join(&config.overlays[source_overlay as usize].module.relocations)
    } else if source_autoload >= 0 {
        config_dir.join(&config.autoloads[source_autoload as usize].module.relocations)
    } else {
        config_dir.join(&config.main_module.relocations)
    };

    let mut relocations = Relocations::from_file(&relocs_path)?;
    let reloc = relocations.get_mut(from).context("Relocation not found")?;
    reloc.set_module(RelocationModule::Overlay { id: target_overlay });
    relocations.to_file(&relocs_path)?;

    Ok(())
}
