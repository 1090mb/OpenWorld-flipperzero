/**
 * @file asset_pack_manager.h
 * Custom Asset Pack Manager for OpenWorld Flipper Zero
 * 
 * Allows users to load custom asset packs from /ext/asset_packs/
 * Asset packs can contain:
 * - Custom dolphin animations
 * - Custom icons
 * - Custom sounds
 * - Custom themes
 */
#pragma once

#include <furi.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSET_PACK_BASE_PATH EXT_PATH("asset_packs")
#define ASSET_PACK_MANIFEST_FILE "manifest.txt"

typedef struct AssetPack AssetPack;

typedef enum {
    AssetPackTypeAnimations,
    AssetPackTypeIcons,
    AssetPackTypeSounds,
    AssetPackTypeTheme,
    AssetPackTypeCount,
} AssetPackType;

/**
 * Allocate asset pack manager
 * @return AssetPack instance
 */
AssetPack* asset_pack_alloc(void);

/**
 * Free asset pack manager
 * @param asset_pack AssetPack instance
 */
void asset_pack_free(AssetPack* asset_pack);

/**
 * List available asset packs
 * @param asset_pack AssetPack instance
 * @return Number of asset packs found
 */
uint32_t asset_pack_list_available(AssetPack* asset_pack);

/**
 * Get asset pack name by index
 * @param asset_pack AssetPack instance
 * @param index Index of asset pack
 * @return Asset pack name string, NULL if index out of bounds
 */
const char* asset_pack_get_name(AssetPack* asset_pack, uint32_t index);

/**
 * Load an asset pack by index
 * @param asset_pack AssetPack instance
 * @param index Index of asset pack to load
 * @return true if loaded successfully
 */
bool asset_pack_load(AssetPack* asset_pack, uint32_t index);

/**
 * Unload current asset pack and restore defaults
 * @param asset_pack AssetPack instance
 */
void asset_pack_unload(AssetPack* asset_pack);

/**
 * Check if an asset pack is currently loaded
 * @param asset_pack AssetPack instance
 * @return true if an asset pack is loaded
 */
bool asset_pack_is_loaded(AssetPack* asset_pack);

/**
 * Get the path to an asset in the current pack
 * @param asset_pack AssetPack instance
 * @param asset_name Name of the asset
 * @param path_out Output string for the full path
 * @return true if asset exists
 */
bool asset_pack_get_asset_path(AssetPack* asset_pack, const char* asset_name, FuriString* path_out);

#ifdef __cplusplus
}
#endif
