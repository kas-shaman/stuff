
#pragma once

#include "utility/math.h"
#include "platform/interfaces.h"

namespace voxel {
    struct Voxel {
        std::int16_t positionX, positionY, positionZ, reserved;
        std::uint8_t scaleX, scaleY, scaleZ, colorIndex;
    };
    struct Frame {
        std::vector<Voxel> voxels;
    };
    
    // Load *.vox at fullPath.
    // Center of model is at {sizeX / 2, 0, sizeZ / 2}.
    // @offset is added to voxel's positions
    //
    std::vector<Frame> loadModel(const std::shared_ptr<platform::Platform> &platform, const char *fullPath, const math::vector3f &offset);

    // Load 256x1 RGBA *.png at fullPath (1024 bytes data).
    //
    std::shared_ptr<platform::Texture2D> loadPalette(
        const std::shared_ptr<platform::Platform> &platform,
        const std::shared_ptr<platform::RenderingDevice> &renderingDevice,
        const char *fullPath
    );
}
