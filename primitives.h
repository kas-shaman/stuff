
#pragma once

#include "utility/math.h"
#include "platform/interfaces.h"

// TODO: text, textured rect

class Primitives {
public:
    Primitives(const std::shared_ptr<platform::RenderingDevice> &renderingDevice) : _renderingDevice(renderingDevice) {}
    
    inline void drawLine(const math::vector3f &p1, const math::vector3f &p2, const math::color &rgba) {
        static const char *lineShader = R"(
            const {
                position[2] : float4
                color : float4
            }
            vssrc {
                out_position = _transform(position[vertex_ID], _viewProjMatrix);
            }
            fssrc {
                out_color = color;
            }
        )";
        
        struct {
            math::vector4f position1;
            math::vector4f position2;
            math::vector4f color;
        }
        lineData {
            {p1, 1},
            {p2, 1},
            rgba
        };
        
        if (_lineShader == nullptr) {
            _lineShader = _renderingDevice->createShader(lineShader, {
                {"ID", platform::ShaderInput::Format::VERTEX_ID}
            });
        }
        
        _renderingDevice->applyShader(_lineShader, &lineData);
        _renderingDevice->drawGeometry(2, platform::Topology::LINES);
    }
    
    inline void drawCircleXZ(const math::vector3f &position, float radius, const math::color &rgba) {
        static const char *circleShader = R"(
            const {
                position_radius : float4
                color : float4
            }
            vssrc {
                float4 point = float4(position_radius.xyz, 1);
                point.x = point.x + position_radius.w * _cos(6.2831853 * float(vertex_ID) / 36.0);
                point.z = point.z + position_radius.w * _sin(6.2831853 * float(vertex_ID) / 36.0);
                out_position = _transform(point, _viewProjMatrix);
            }
            fssrc {
                out_color = color;
            }
        )";
        
        struct {
            math::vector4f positionRadius;
            math::vector4f color;
        }
        circleData {
            {position, radius},
            rgba
        };
        
        if (_circleShader == nullptr) {
            _circleShader = _renderingDevice->createShader(circleShader, {
                {"ID", platform::ShaderInput::Format::VERTEX_ID}
            });
        }
        
        _renderingDevice->applyShader(_circleShader, &circleData);
        _renderingDevice->drawGeometry(37, platform::Topology::LINESTRIP);
    }
    
    inline void drawCylinderXZ(const math::vector3f &position, float radius, float height, const math::color &rgba) {
        
    }
    
    inline void drawCube(const math::vector3f &p1, const math::vector3f &p2, const math::color &rgba) {
        
    }
    
    inline void drawAxis() {
        for (int i = -10; i <= 10; i++) {
            drawLine({float(i), 0, -10}, {float(i), 0, 10}, {0.2f, 0.2f, 0.2f, 1});
            drawLine({-10, 0, float(i)}, {10, 0, float(i)}, {0.2f, 0.2f, 0.2f, 1});
        }

        drawLine({100, 0, 0}, {0, 0, 0}, {1, 0, 0, 1});
        drawLine({0, 100, 0}, {0, 0, 0}, {0, 1, 0, 1});
        drawLine({0, 0, 100}, {0, 0, 0}, {0, 0, 1, 1});
    }
    
protected:
    std::shared_ptr<platform::RenderingDevice> _renderingDevice;
    std::shared_ptr<platform::Shader> _lineShader;
    std::shared_ptr<platform::Shader> _circleShader;
};
