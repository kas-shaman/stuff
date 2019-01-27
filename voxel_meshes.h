
#pragma once

#include <cstdint>
#include <memory>
#include <functional>

#include "utility/math.h"
#include "utility/common.h"

#include "platform/interfaces.h"

namespace voxel {
    class VoxelMesh : public utility::NonCopyable, public utility::NonMovable {
    public:
        void setTransform(const math::transform3f &fullTransform);
        void playAnimation(const char *name, std::function<void(VoxelMesh&)> &&finished);

    protected:
        VoxelMesh() = default;
    };

    class VoxelMeshes : public utility::NonCopyable, public utility::NonMovable {
    public:
        std::shared_ptr<VoxelMesh> loadMesh(const char *fullFolderPath);
        void updateAndDraw(float dtSec);

    protected:
        VoxelMeshes() = default;
    };

    std::shared_ptr<VoxelMeshes> makeVoxelMeshes(
        const std::shared_ptr<platform::Platform> &platform,
        const std::shared_ptr<platform::RenderingDevice> &renderingDevice,
        const std::shared_ptr<platform::Texture2D> &palette
    );
}
