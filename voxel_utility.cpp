
#include "voxel_utility.h"

namespace lib {
#include "lib/upng.h"
#include "lib/upng.c"
}

#include <array>

namespace voxel {
    std::vector<Frame> loadModel(const std::shared_ptr<platform::Platform> &platform, const char *fullPath, const math::vector3f &offset) {
        const std::int32_t version = 150;

        std::vector<Frame> result;
        std::unique_ptr<std::uint8_t []> voxData;
        std::size_t voxSize = 0;
        
        if (platform->loadFile(fullPath, voxData, voxSize)) {
            std::uint8_t *data = voxData.get();
            
            if (memcmp(data, "VOX ", 4) == 0 && *(std::int32_t *)(data + 4) == version) {
                // skip bytes of main chunk to start of the first child ('PACK')
                data += 20;
                
                if (memcmp(data, "PACK", 4) == 0) {
                    std::int32_t modelCount = *(std::int32_t *)(data + 12);
                    
                    data += 16;
                    result.resize(modelCount);
                    
                    for (std::int32_t i = 0; i < modelCount; i++) {
                        if (memcmp(data, "SIZE", 4) == 0) {
                            std::int16_t centeringZ = *(std::int8_t *)(data + 12) / 2;
                            std::int16_t centeringX = *(std::int8_t *)(data + 16) / 2;
                            
                            data += 24;
                         
                            if (memcmp(data, "XYZI", 4) == 0) {
                                std::int32_t voxelCount = *(std::int32_t *)(data + 12);
                                
                                data += 16;
                                result[i].voxels.resize(voxelCount);
                                
                                for (std::int32_t c = 0; c < voxelCount; c++) {
                                    result[i].voxels[c].positionZ = *(std::int8_t *)(data + c * 4 + 0) - centeringZ;
                                    result[i].voxels[c].positionX = *(std::int8_t *)(data + c * 4 + 1) - centeringX;
                                    result[i].voxels[c].positionY = *(std::int8_t *)(data + c * 4 + 2);
                                    result[i].voxels[c].colorIndex = *(std::uint8_t *)(data + c * 4 + 3) - 1;
                                    
                                    // TODO: voxel mesh optimization
                                    result[i].voxels[c].scaleX = 1;
                                    result[i].voxels[c].scaleY = 1;
                                    result[i].voxels[c].scaleZ = 1;
                                }
                                
                                data += voxelCount * 4;
                            }
                            else {
                                platform->logError("[voxel::loadModel] XYZI[%d] chunk is not found in '%s'", i, fullPath);
                                break;
                            }
                        }
                        else {
                            platform->logError("[voxel::loadModel] SIZE[%d] chunk is not found in '%s'", i, fullPath);
                            break;
                        }
                    }
                }
                else {
                    platform->logError("[voxel::loadModel] PACK chunk is not found in '%s'", fullPath);
                }
            }
            else {
                platform->logError("[voxel::loadModel] Incorrect vox-header in '%s'", fullPath);
            }
        }
        else {
            platform->logError("[voxel::loadModel] Unable to find file '%s'", fullPath);
        }
        
        return result;
    }
    
    std::shared_ptr<platform::Texture2D> loadPalette(
        const std::shared_ptr<platform::Platform> &platform,
        const std::shared_ptr<platform::RenderingDevice> &renderingDevice,
        const char *fullPath
    ) {
        std::shared_ptr<platform::Texture2D> result;
        std::unique_ptr<std::uint8_t []> paletteData;
        std::size_t paletteSize;
        
        if (platform->loadFile(fullPath, paletteData, paletteSize)) {
            lib::upng_t* upng = lib::upng_new_from_bytes(paletteData.get(), paletteSize);
            
            if (upng != nullptr) {
                if (*reinterpret_cast<const unsigned *>(paletteData.get()) == 0x474E5089 && lib::upng_decode(upng) == lib::UPNG_EOK) {
                    if (lib::upng_get_format(upng) == lib::UPNG_RGBA8 && lib::upng_get_width(upng) == 256 && lib::upng_get_height(upng) == 1) {
                        result = renderingDevice->createTexture(platform::Texture2D::Format::RGBA8UN, 256, 1, {lib::upng_get_buffer(upng)});
                        
                        if (result == nullptr) {
                            platform->logError("[voxel::loadTexture] Unable to create platform::Texture2D for '%s'", fullPath);
                        }
                    }
                    else {
                        platform->logError("[voxel::loadTexture] '%s' is not 256x1 RGBA png file", fullPath);
                    }
                }
                else {
                    platform->logError("[voxel::loadTexture] '%s' is not a valid png file", fullPath);
                }

                upng_free(upng);
            }
        }
        else {
            platform->logError("[voxel::loadTexture] '%s' is not found", fullPath);
        }

        return result;
    }
}
