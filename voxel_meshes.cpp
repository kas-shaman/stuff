
#include "voxel_meshes.h"
#include "voxel_utility.h"

#include <unordered_map>

namespace {
    static constexpr uint32_t HALF_CUBE_VERTEX_COUNT = 12;

    struct VoxelMeshShaderConst {
        math::vector4f axis[3] = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
        };
        math::vector4f cube[HALF_CUBE_VERTEX_COUNT] = {
            {-0.5f, 0.5f, 0.5f, 1.0f},
            {-0.5f, -0.5f, 0.5f, 1.0f},
            {0.5f, 0.5f, 0.5f, 1.0f},
            {0.5f, -0.5f, 0.5f, 1.0f},
            {0.5f, -0.5f, 0.5f, 1.0f},
            {0.5f, 0.5f, 0.5f, 1.0f},
            {0.5f, -0.5f, -0.5f, 1.0f},
            {0.5f, 0.5f, -0.5f, 1.0f},
            {0.5f, 0.5f, -0.5f, 1.0f},
            {0.5f, 0.5f, 0.5f, 1.0f},
            {-0.5f, 0.5f, -0.5f, 1.0f},
            {-0.5f, 0.5f, 0.5f, 1.0f},
        };
    }
    _voxelMeshShaderConst;
    
    const char *_voxelMeshShader = R"(
        prmnt {
            axis[3] : float4
            cube[12] : float4
        }
        inter {
            texcoord : float2
        }
        vssrc {
            float4 position = float4(instance_position.xyz, 1.0);
            float3 camSign = _sign(_cameraPosition.xyz - position.xyz);
            float4 cube_position = float4(camSign, 0.0) * cube[vertex_ID] + position;
            out_position = _transform(cube_position, _viewProjMatrix);
            inter.texcoord = float2(float(instance_scale_color.w) / 255.0, 0);
        }
        fssrc {
            out_color = _tex2d(0, inter.texcoord);
        }
    )";
}

namespace voxel {
    class VoxelMeshImp : public VoxelMesh {
    public:
        struct Frame {
            std::shared_ptr<platform::StructuredData> voxels;
        };
        
        struct Animation {
            std::size_t firstFrame;
            std::size_t lastFrame;
            float frameRate;
        };
        
        VoxelMeshImp(
            const std::shared_ptr<platform::Platform> &platform,
            const std::shared_ptr<platform::RenderingDevice> &renderingDevice,
            const std::vector<voxel::Frame> &frames,
            std::unordered_map<std::string, Animation> &&animations
        )
        : _platform(platform)
        , _animations(std::move(animations))
        {
            _frames.reserve(frames.size());
            
            for (auto &frame : frames) {
                _frames.emplace_back(Frame {renderingDevice->createData(&frame.voxels[0], uint32_t(frame.voxels.size()), sizeof(voxel::Voxel))});
            }
        }
        
        ~VoxelMeshImp() {
        
        }
        
        void setTransform(const math::transform3f &fullTransform) {
            _transform = fullTransform;
        }
        
        void playAnimation(const char *name, std::function<void(VoxelMesh&)> &&finished) {
            auto index = _animations.find(name);
            if (index != _animations.end()) {
                _currentAnimation = &index->second;
                _currentFrame = _currentAnimation->firstFrame;
                _time = 0.0f;
                _lastFrame = 0;
                _finished = std::move(finished);
            }
            else {
                _currentAnimation = nullptr;
            }
        }
        
        void updateAnimation(float dtSec) {
            if (_currentAnimation) {
                std::size_t frame = std::size_t(_time * _currentAnimation->frameRate);
                
                if (frame != _lastFrame) {
                    if (_currentFrame == _currentAnimation->lastFrame) {
                        _currentFrame = _currentAnimation->firstFrame;
                        
                        if (_finished) {
                            _finished(*this);
                        };
                        
                        _currentAnimation = nullptr;
                        //_time -= float(_currentAnimation->lastFrame - _currentAnimation->firstFrame + 1) / _currentAnimation->frameRate;
                        //frame = std::size_t(_time * _currentAnimation->frameRate);
                    }
                    else {
                        _currentFrame++;
                    }
                    
                    _lastFrame = frame;
                }
                
                _time += dtSec;
            }
            else {
                _currentFrame = 0;
            }
        }
        
        const std::shared_ptr<platform::StructuredData> &getVoxelData() const {
            return _frames[_currentFrame].voxels;
        }
        
        std::uint32_t getVoxelCount() const {
            return _frames[_currentFrame].voxels->getCount();
        }
        
    private:
        std::shared_ptr<platform::Platform> _platform;
        Animation *_currentAnimation = nullptr;

        std::function<void(VoxelMesh&)> _finished;
        std::unordered_map<std::string, Animation> _animations;
        std::vector<Frame> _frames;

        math::transform3f _transform = math::transform3f::identity();
        float _time = 0.0f;
        
        std::size_t _lastFrame = 0;
        std::size_t _currentFrame = 0;
    };

    void VoxelMesh::setTransform(const math::transform3f &fullTransform) {
        static_cast<VoxelMeshImp *>(this)->setTransform(fullTransform);
    }

    void VoxelMesh::playAnimation(const char *name, std::function<void(VoxelMesh&)> &&finished) {
        static_cast<VoxelMeshImp *>(this)->playAnimation(name, std::move(finished));
    }

    class VoxelMeshesImp : public VoxelMeshes {
    public:
        VoxelMeshesImp(
            const std::shared_ptr<platform::Platform> &platform,
            const std::shared_ptr<platform::RenderingDevice> &renderingDevice,
            const std::shared_ptr<platform::Texture2D> &palette
        ) {
            _platform = platform;
            _renderingDevice = renderingDevice;
            _palette = palette;
            
            _shader = renderingDevice->createShader(
                _voxelMeshShader,
                {{"ID", platform::ShaderInput::Format::VERTEX_ID}},
                {
                    {"position", platform::ShaderInput::Format::SHORT4},
                    {"scale_color", platform::ShaderInput::Format::BYTE4}
                },
                &_voxelMeshShaderConst
            );
        }
        
        ~VoxelMeshesImp() {
        
        }

        std::shared_ptr<VoxelMesh> loadMesh(const char *fullFolderPath) {
            std::string infoPath = std::string(fullFolderPath) + "/model.info";
            std::string modelPath = std::string(fullFolderPath) + "/model.vox";
            std::vector<voxel::Frame> frames = voxel::loadModel(_platform, modelPath.data(), {0, 0, 0});
            std::unordered_map<std::string, VoxelMeshImp::Animation> animations;
            
            if (frames.size()) {
                std::unique_ptr<uint8_t []> infoData;
                std::size_t infoSize;
                
                if (_platform->loadFile(infoPath.data(), infoData, infoSize)) {
                    std::istringstream stream (std::string(reinterpret_cast<const char *>(infoData.get()), infoSize));
                    std::string keyword;
                    
                    while (stream >> keyword) {
                        if (keyword == "animation") {
                            std::string animationName;
                            std::size_t firstFrame, lastFrame;
                            float frameRate;
                            
                            if (stream >> utility::expect<'='> >> utility::quoted(animationName) >> firstFrame >> lastFrame >> frameRate) {
                                animations.emplace(std::move(animationName), VoxelMeshImp::Animation {firstFrame, lastFrame, frameRate});
                            }
                            else {
                                _platform->logError("[VoxelMeshes] Invalid animation '%s' arguments in '%s'", animationName.data(), infoPath.data());
                                break;
                            }
                        }
                        else {
                            _platform->logError("[VoxelMeshes] Unreconized keyword '%s' in '%s'", keyword.data(), infoPath.data());
                            break;
                        }
                    }
                }

                _meshes.emplace_back(std::make_shared<VoxelMeshImp>(_platform, _renderingDevice, frames, std::move(animations)));
                return _meshes.back();
            }

            return nullptr;
        }
        
        void updateAndDraw(float dtSec) {
            _renderingDevice->applyTextures({_palette.get()});
            _renderingDevice->applyShader(_shader);
            
            for (auto &mesh : _meshes) {
                mesh->updateAnimation(dtSec);
                _renderingDevice->drawGeometry(nullptr, mesh->getVoxelData(), HALF_CUBE_VERTEX_COUNT, mesh->getVoxelCount(), platform::Topology::TRIANGLESTRIP);
            }
        }
        
        std::shared_ptr<platform::Platform> _platform;
        std::shared_ptr<platform::RenderingDevice> _renderingDevice;
        std::shared_ptr<platform::Shader> _shader;
        std::vector<std::shared_ptr<VoxelMeshImp>> _meshes;
        std::shared_ptr<platform::Texture2D> _palette;
    };

    std::shared_ptr<VoxelMesh> VoxelMeshes::loadMesh(const char *fullFolderPath) {
        return static_cast<VoxelMeshesImp *>(this)->loadMesh(fullFolderPath);
    }

    void VoxelMeshes::updateAndDraw(float dt) {
        static_cast<VoxelMeshesImp *>(this)->updateAndDraw(dt);
    }

    std::shared_ptr<VoxelMeshes> makeVoxelMeshes(
        const std::shared_ptr<platform::Platform> &platform,
        const std::shared_ptr<platform::RenderingDevice> &renderingDevice,
        const std::shared_ptr<platform::Texture2D> &palette
    ) {
        return std::make_shared<VoxelMeshesImp>(platform, renderingDevice, palette);
    }
}
