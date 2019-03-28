
#include "tool.h"

#include "common.h"
#include "viewpanel.h"
#include "camera.h"
#include "viewport.h"

#define HANDLE_SIZE 100

QSizeF Tool::m_viewPanelSize;

glm::mat4
Tool::getAxialBasis( unsigned int axis )
{
    const float m[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    unsigned int x = (axis+2)%3;
    unsigned int y = axis;
    unsigned int z = (axis+1)%3;
    return glm::mat4( m[x], m[3+x], m[6+x], 0,
                      m[y], m[3+y], m[6+y], 0,
                      m[z], m[3+z], m[6+z], 0,
                         0,      0,      0, 1 );
}

float
Tool::getHandleSize( const glm::vec3 &center ) const
{
    glm::vec3 c(center);
    Camera *camera = m_panel->m_viewport->getCamera();
    float distance = glm::length( c - camera->getPosition() );
    glm::vec2 uv = camera->getProjection( c );
    glm::vec3 ray = camera->getCameraRay( uv + glm::vec2(0.f, HANDLE_SIZE/(float)m_panel->height()) );
    glm::vec3 point = camera->getPosition() + distance*ray;
    return glm::length( point-c );
}

glm::vec3
Tool::getAxialColor( unsigned int axis )
{
    switch ( axis ) {
    case 0:
        return glm::vec3( 186/255., 70/255., 85/255. );
    case 1:
        return glm::vec3( 91/255., 180/255., 71/255. );
    case 2:
        return glm::vec3( 79/255., 79/255., 190/255. );
    default:
        return glm::vec3( 190/255., 190/255., 69/255. );
    }
}
