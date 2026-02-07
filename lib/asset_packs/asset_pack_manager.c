/**
 * @file asset_pack_manager.c
 * Custom Asset Pack Manager Implementation
 */

#include "asset_pack_manager.h"
#include <m-array.h>
#include <storage/storage.h>

#define ASSET_PACK_MAX_NAME_LENGTH 64
#define ASSET_PACK_MAX_COUNT 32

typedef struct {
    char name[ASSET_PACK_MAX_NAME_LENGTH];
    char path[256];
} AssetPackInfo;

ARRAY_DEF(AssetPackInfoArray, AssetPackInfo, M_POD_OPLIST);

struct AssetPack {
    AssetPackInfoArray_t packs;
    int32_t current_pack_index;
    FuriString* current_pack_path;
};

AssetPack* asset_pack_alloc(void) {
    AssetPack* asset_pack = malloc(sizeof(AssetPack));
    AssetPackInfoArray_init(asset_pack->packs);
    asset_pack->current_pack_index = -1;
    asset_pack->current_pack_path = furi_string_alloc();
    return asset_pack;
}

void asset_pack_free(AssetPack* asset_pack) {
    furi_assert(asset_pack);
    AssetPackInfoArray_clear(asset_pack->packs);
    furi_string_free(asset_pack->current_pack_path);
    free(asset_pack);
}

uint32_t asset_pack_list_available(AssetPack* asset_pack) {
    furi_assert(asset_pack);

    // Clear existing list
    AssetPackInfoArray_reset(asset_pack->packs);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);

    // Create base path if it doesn't exist
    storage_common_mkdir(storage, ASSET_PACK_BASE_PATH);

    if(storage_dir_open(dir, ASSET_PACK_BASE_PATH)) {
        FileInfo file_info;
        char name[256];

        while(storage_dir_read(dir, &file_info, name, sizeof(name))) {
            if(file_info_is_dir(&file_info) && name[0] != '.') {
                AssetPackInfo info;
                snprintf(info.name, sizeof(info.name), "%s", name);
                snprintf(info.path, sizeof(info.path), "%s/%s", ASSET_PACK_BASE_PATH, name);
                
                // Check if manifest exists
                FuriString* manifest_path = furi_string_alloc();
                furi_string_printf(manifest_path, "%s/%s", info.path, ASSET_PACK_MANIFEST_FILE);
                
                FileInfo manifest_info;
                if(storage_common_stat(storage, furi_string_get_cstr(manifest_path), &manifest_info) == FSE_OK) {
                    AssetPackInfoArray_push_back(asset_pack->packs, info);
                }
                
                furi_string_free(manifest_path);
            }
        }
        storage_dir_close(dir);
    }

    storage_file_free(dir);
    furi_record_close(RECORD_STORAGE);

    return AssetPackInfoArray_size(asset_pack->packs);
}

const char* asset_pack_get_name(AssetPack* asset_pack, uint32_t index) {
    furi_assert(asset_pack);
    
    if(index >= AssetPackInfoArray_size(asset_pack->packs)) {
        return NULL;
    }
    
    const AssetPackInfo* info = AssetPackInfoArray_cget(asset_pack->packs, index);
    return info->name;
}

bool asset_pack_load(AssetPack* asset_pack, uint32_t index) {
    furi_assert(asset_pack);
    
    if(index >= AssetPackInfoArray_size(asset_pack->packs)) {
        return false;
    }
    
    const AssetPackInfo* info = AssetPackInfoArray_cget(asset_pack->packs, index);
    furi_string_set_str(asset_pack->current_pack_path, info->path);
    asset_pack->current_pack_index = index;
    
    return true;
}

void asset_pack_unload(AssetPack* asset_pack) {
    furi_assert(asset_pack);
    
    asset_pack->current_pack_index = -1;
    furi_string_reset(asset_pack->current_pack_path);
}

bool asset_pack_is_loaded(AssetPack* asset_pack) {
    furi_assert(asset_pack);
    return asset_pack->current_pack_index >= 0;
}

bool asset_pack_get_asset_path(AssetPack* asset_pack, const char* asset_name, FuriString* path_out) {
    furi_assert(asset_pack);
    furi_assert(asset_name);
    furi_assert(path_out);
    
    if(!asset_pack_is_loaded(asset_pack)) {
        return false;
    }
    
    furi_string_printf(path_out, "%s/%s", furi_string_get_cstr(asset_pack->current_pack_path), asset_name);
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FileInfo file_info;
    bool exists = (storage_common_stat(storage, furi_string_get_cstr(path_out), &file_info) == FSE_OK);
    furi_record_close(RECORD_STORAGE);
    
    return exists;
}
