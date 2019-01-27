
#pragma once

#include "camera.h"

class OrbitCameraController {
public:
    OrbitCameraController(const std::shared_ptr<platform::Platform> &platform, const std::shared_ptr<Camera> &camera) : _platform(platform), _camera(camera) {
        setEnabled(true);
        _camera->setLookAtByRight(_center + _orbit, _center, math::vector3f(0, 1, 0).cross(_orbit));
    }
    
    ~OrbitCameraController() {
        setEnabled(false);
    }
    
    void setEnabled(bool enabled) {
        if (enabled) {
            _touchEventHandlersToken = _platform->addTouchEventHandlers(
                [this](const platform::TouchEventArgs &args) {
                    _lockedTouchID = args.touchID;
                    _lockedTouchCoordinates = {args.coordinateX, args.coordinateY};
                },
                [this](const platform::TouchEventArgs &args) {
                    if (args.touchID == _lockedTouchID) {
                        float dx = args.coordinateX - _lockedTouchCoordinates.x;
                        float dy = args.coordinateY - _lockedTouchCoordinates.y;

                        _orbit.xz = _orbit.xz.rotated(dx / 100.0f);
                        math::vector3f right = math::vector3f(0, 1, 0).cross(_orbit);
                        _orbit = _orbit.rotated(right, -dy / 100.0f);
                        
                        _camera->setLookAtByRight(_center + _orbit, _center, right);
                        _lockedTouchCoordinates = {args.coordinateX, args.coordinateY};
                    }
                },
                [this](const platform::TouchEventArgs &args) {
                    _lockedTouchID = 0;
                }
            );
        }
        else {
            _platform->removeEventHandlers(_touchEventHandlersToken);
            _touchEventHandlersToken = nullptr;
        }
    }
    
protected:
    std::shared_ptr<platform::Platform> _platform;
    std::shared_ptr<Camera> _camera;
    
    std::size_t _lockedTouchID;
    math::vector2f _lockedTouchCoordinates;

    math::vector3f _center = {0, 0, 0};
    math::vector3f _orbit = {50, 20, 50};

    platform::EventHandlersToken _touchEventHandlersToken;
};
