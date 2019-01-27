#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#include "platform/interfaces.h"
#include "utility/math.h"

#include "camera.h"
#include "primitives.h"

#include "orbit_camera_controller.h"
#include "voxel_meshes.h"
#include "voxel_utility.h"

int main(int argc, char * argv[]) {
    auto platform = platform::getPlatformInstance();
    auto renderingDevice = platform::getRenderingDeviceInstance(platform);
    auto primitives = std::make_shared<Primitives>(renderingDevice);

    auto camera = std::make_shared<Camera>(platform);
    auto cameraController = std::make_shared<OrbitCameraController>(platform, camera);
    
    auto voxelPalette = voxel::loadPalette(platform, renderingDevice, "data/palette.png");
    auto voxelMeshes = voxel::makeVoxelMeshes(platform, renderingDevice, voxelPalette);

    auto voxelMesh = voxelMeshes->loadMesh("data/knight");
    
    std::size_t touchID = 0;
    platform->addTouchEventHandlers(
        [&](const platform::TouchEventArgs &args) {
            touchID = args.touchID;
        },
        [&](const platform::TouchEventArgs &args) {
            if (args.touchID == touchID) {
                touchID = 0;
            }
        },
        [&](const platform::TouchEventArgs &args) {
            if (args.touchID == touchID) {
                voxelMesh->playAnimation("walk", nullptr);
                touchID = 0;
            }
        }
    );

    platform->run([&](float dtSec) {
        renderingDevice->updateCameraTransform(camera->getPosition().flat3, camera->getForwardDirection().flat3, camera->getVPMatrix().flat16);
        renderingDevice->prepareFrame();
        primitives->drawAxis();

        voxelMeshes->updateAndDraw(dtSec);

        renderingDevice->presentFrame(dtSec);
    });
}







