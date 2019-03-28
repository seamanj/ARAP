//#include <Windows.h>

#include <GL/glew.h>
#include <GL/gl.h>
#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "movetool.h"

#include "common.h"
#include "scene.h"
#include "scenenode.h"
#include "scenenodeiterator.h"
#include "picker.h"
#include "userinput.h"
#include "viewpanel.h"
#include "camera.h"
#include "viewport.h"

#include "mymath.h"
#include <memory>
using std::dynamic_pointer_cast;
using glm::vec3;

MoveTool::MoveTool( ViewPanel *panel,Type t )
    : SelectionTool(panel,t),
      m_axisSelection(Picker::NO_PICK),
      m_active(false),
      m_moving(false),
      m_center(0,0,0),
      m_scale(1.f),
      m_vbo(0),
      count(0)
{
}

MoveTool::~MoveTool()
{
    deleteVBO();
}

void
MoveTool::update()
{
    if ( (m_active = SelectionTool::hasMovableSelection(m_center)) ) {
        m_scale = Tool::getHandleSize( m_center );
    }
}

void
MoveTool::renderAxis( unsigned int i ) const
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glm::mat4 translate = glm::translate( glm::mat4(1.f), glm::vec3(m_center) );
    glm::mat4 basis = glm::scale( Tool::getAxialBasis(i), glm::vec3(m_scale) );
    glMultMatrixf( glm::value_ptr(translate*basis) );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof(glm::vec3), (void*)(0) );
    glLineWidth( 2.f );
    glDrawArrays( GL_LINES, 0, 2 );
    glDrawArrays( GL_TRIANGLES, 2, m_vboSize-(2+24) );
    glDisableClientState( GL_VERTEX_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glPopMatrix();
}

void
MoveTool::renderCenter() const
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glm::mat4 translate = glm::translate( glm::mat4(1.f), glm::vec3(m_center) );
    glm::mat4 scale = glm::scale( glm::mat4(1.f), glm::vec3(m_scale) );
    glMultMatrixf( glm::value_ptr(translate*scale) );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof(vec3), (void*)(0) );
    glDrawArrays( GL_QUADS, m_vboSize-24, 24 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glPopMatrix();
}

void
MoveTool::render()
{
    if ( m_active ) {

        if ( !hasVBO() ) buildVBO();

        glPushAttrib( GL_DEPTH_BUFFER_BIT );
        glDisable( GL_DEPTH_TEST );
        glPushAttrib( GL_LIGHTING_BIT );
        glDisable( GL_LIGHTING );
        glPushAttrib( GL_COLOR_BUFFER_BIT );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        for ( unsigned int i = 0; i < 3; ++i ) {
            glColor3fv( glm::value_ptr(Tool::getAxialColor((i==m_axisSelection)?3:i)) );
            renderAxis( i );
        }
        glColor3fv( glm::value_ptr(getAxialColor(3)) );
        renderCenter();
        glPopAttrib();
        glPopAttrib();
        glPopAttrib();
    }
    SelectionTool::render();
}

unsigned int
MoveTool::getAxisPick() const
{
    unsigned int pick = Picker::NO_PICK;
    if ( m_active ) {
        m_panel->m_viewport->loadPickMatrices( UserInput::mousePos(), 6.f );
        Picker picker( 4 );
        for ( unsigned int i = 0; i < 3; ++i ) {
            picker.setObjectIndex( i );
            renderAxis( i );
        }
        picker.setObjectIndex( 4 );
        renderCenter();
        pick = picker.getPick();
        m_panel->m_viewport->popMatrices();
    }
    return pick;
}

void
MoveTool::mousePressed()
{
    if ( m_active ) {
        m_axisSelection = getAxisPick();
        m_moving = ( m_axisSelection != Picker::NO_PICK );
        if ( m_axisSelection == Picker::NO_PICK ) {
            SelectionTool::mousePressed();
        }
    } else {
        SelectionTool::mousePressed();
    }
    update();
}

float
MoveTool::intersectAxis( const glm::ivec2 &mouse ) const
{
    glm::vec2 uv = glm::vec2( (float)mouse.x/m_panel->width(), (float)mouse.y/m_panel->height() );
    vec3 direction = m_panel->m_viewport->getCamera()->getCameraRay( uv );
    vec3 origin = m_panel->m_viewport->getCamera()->getPosition();
    unsigned int majorAxis = ::majorAxis(direction);
    int axis = majorAxis;
    if ( majorAxis == m_axisSelection ) {
        axis = ( majorAxis == 0 ) ? 1 : 0;
    }
    float t = (m_center[axis]-origin[axis])/direction[axis];
    vec3 point = origin + t*direction;
    vec3 a = vec3(0,0,0); a[m_axisSelection] = 1.f;
    return glm::dot( a, point-m_center );

}



void
MoveTool::mouseMoved()
{
    if ( m_moving ) {
        const glm::ivec2 &p0 = UserInput::mousePos() - UserInput::mouseMove();
        const glm::ivec2 &p1 = UserInput::mousePos();
        glm::mat4 transform = glm::mat4(1.f);
        if ( m_axisSelection < 3 ) {
            float t0 = intersectAxis( p0 );
            float t1 = intersectAxis( p1 );
            float t = t1-t0;
            glm::vec3 translate = glm::vec3(0,0,0); translate[m_axisSelection] = t;
            transform = glm::translate( glm::mat4(1.f), translate );
        } else {
            Camera *camera = m_panel->m_viewport->getCamera();
            float depth = glm::dot( (m_center - vec3(camera->getPosition())), camera->getLook() );
            vec3 ray0 = camera->getCameraRay( glm::vec2(p0.x/(float)m_panel->width(),p0.y/(float)m_panel->height()) );
            float t0 = depth / glm::dot( ray0, camera->getLook() );
            vec3 ray1 = camera->getCameraRay( glm::vec2(p1.x/(float)m_panel->width(),p1.y/(float)m_panel->height()) );
            float t1 = depth / glm::dot( ray1, camera->getLook() );
            glm::vec3 translate = t1*ray1-t0*ray0;
            transform = glm::translate( glm::mat4(1.f), translate );
        }
        for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
            if ( (*it)->hasRenderable() && (*it)->getRenderable()->isSelected() && (*it)->isMovable() ) 
            {
                
                if( (*it)->isParticle() )
                {
                     //dynamic_pointer_cast<Particle>((*it)->getRenderable())->setDragged(true);
                    (*it)->applyTransformationOnRendable( transform );
                }
                else
                    (*it)->applyTransformation( transform );
                    
            }
        }
    }
    else
    {
        SelectionTool::mouseMoved();
    }
    update();
    count++;
    if(count % 16 == 15)
    {
        count = 0;
        if( m_moving )
        {
            emit deformPolyhedron();
        }
    }

}

void
MoveTool::mouseReleased()
{
    if( m_moving )
    {
        emit deformPolyhedron();
    }
    m_axisSelection = Picker::NO_PICK;
    m_moving = false;
    
//    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it )
//    {
        
//        if( (*it)->isParticle() )
//        {
//            dynamic_pointer_cast<Particle>((*it)->getRenderable())->setDragged(false);
//        }
//    }
    
    SelectionTool::mouseReleased();
    update();

}

bool
MoveTool::hasVBO() const
{
    return m_vbo > 0 && glIsBuffer( m_vbo );
}

void
MoveTool::buildVBO()
{
    deleteVBO();

    QVector<vec3> data;

    // Axis
    data += vec3( 0, 0, 0 );
    data += vec3( 0, 1, 0 );

    // Cone
    static const int resolution = 60;
    static const float dTheta = 2.f*M_PI/resolution;
    static const float coneHeight = 0.1f;
    static const float coneRadius = 0.05f;
    for ( int i = 0; i < resolution; ++i ) {
        data += vec3( 0, 1, 0 );
        float theta0 = i*dTheta;
        float theta1 = (i+1)*dTheta;
        data += (vec3(0,1-coneHeight,0)+coneRadius*vec3(cosf(theta0),0,-sinf(theta0)));
        data += (vec3(0,1-coneHeight,0)+coneRadius*vec3(cosf(theta1),0,-sinf(theta1)));
    }

    // Cube
    static const float s = 0.05f;
    data += vec3( -s, s, -s );
    data += vec3( -s, -s, -s );
    data += vec3( -s, -s, s );
    data += vec3( -s, s, s );
    data += vec3( s, s, s );
    data += vec3( s, -s, s );
    data += vec3( s, -s, -s );
    data += vec3( s, s, -s );
    data += vec3( -s, s, s );
    data += vec3( -s, -s, s );
    data += vec3( s, -s, s );
    data += vec3( s, s, s );
    data += vec3( s, s, -s );
    data += vec3( s, -s, -s );
    data += vec3( -s, -s, -s );
    data += vec3( -s, s, -s );
    data += vec3( -s, s, -s );
    data += vec3( -s, s, s );
    data += vec3( s, s, s );
    data += vec3( s, s, -s );
    data += vec3( s, s, -s );
    data += vec3( s, s, s );
    data += vec3( -s, s, s );
    data += vec3( -s, s, -s );

    glewInit();
    
    glGenBuffers( 1, &m_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
    glBufferData( GL_ARRAY_BUFFER, data.size()*sizeof(vec3), data.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    m_vboSize = data.size();
}

void
MoveTool::deleteVBO()
{
    if ( m_vbo > 0 ) {
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
        if ( glIsBuffer(m_vbo) ) glDeleteBuffers( 1, &m_vbo );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        m_vbo = 0;
    }
}
