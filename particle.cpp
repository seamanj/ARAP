
//#include <Windows.h>

#include "particle.h"
#include "uisettings.h"
#define GLM_SWIZZLE 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <particlesystem.h>
#include <GL/gl.h>
Particle::Particle()
    : m_ROI(false), m_control(false), m_parent(NULL)
{
}
Particle::Particle(int order, Vertex position, Normal normal, ParticleSystem* parent, bool ROI, bool control)
    : m_order(order), m_position(position), m_normal(normal), m_parent(parent), m_ROI(ROI), m_control(control)
{

}
Particle::~Particle()
{
    m_parent = NULL;
}

void Particle::render()
{
    glPushAttrib( GL_LIGHTING_BIT );
    glEnable( GL_LIGHTING );
    glm::vec4 color;
    if( m_control )
        color = ( m_selected ) ? glm::mix(  UiSettings::controlColor(), UiSettings::particleSelectionColor(), 0.5f ) : UiSettings::controlColor();
    else if( m_ROI )
        color = ( m_selected ) ? glm::mix(  UiSettings::ROIColor(), UiSettings::particleSelectionColor(), 0.5f ) : UiSettings::ROIColor();
    else
        color = ( m_selected ) ? glm::mix(  glm::vec4(0.4f, 0.4f, 0.4f, 1.f), UiSettings::particleSelectionColor(), 0.5f ) : glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
    
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(color*0.2f) );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(color) );
    glEnable( GL_POINT_SMOOTH );
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glNormal3fv(glm::value_ptr(m_normal));
    glVertex3fv(glm::value_ptr(m_position));
    glEnd();
    glPopAttrib();
}

void 
Particle::applyTransformation( const glm::mat4 &transform )
{
    if( m_control )
    {
        glm::vec4 pos = transform * glm::vec4(m_position, 1.f);
        m_position = glm::vec3(pos.x, pos.y, pos.z);
        //m_parent->updateFromParticles();
        m_parent->set_target_vertex(m_order, m_position);
    }
    
}
void
Particle::renderForPicker()
{


    glPushAttrib( GL_DEPTH_TEST );
    glEnable( GL_DEPTH_TEST );
    glPushAttrib( GL_LIGHTING_BIT );
    glDisable( GL_LIGHTING );
    glColor3f( 1.f, 1.f, 1.f );
    
    glEnable( GL_POINT_SMOOTH );
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glNormal3fv(glm::value_ptr(m_normal));
    glVertex3fv(glm::value_ptr(m_position));
    glEnd();
    
    
    glPopAttrib();
    glPopAttrib();
}
glm::vec3
Particle::getCentroid( const glm::mat4 &ctm )
{

    glm::vec4 point = ctm * glm::vec4( m_position, 1.f );
    return glm::vec3(point.x, point.y, point.z);

}
