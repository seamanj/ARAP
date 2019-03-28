//#include <Windows.h>
#include "scenenode.h"
#include "common.h"
#include <GL/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


SceneNode::SceneNode( Type type )
    : m_parent(NULL),
      m_ctm(1.f),
      m_ctmDirty(true),
      m_transform(1.f),
      m_spRenderable(NULL),
      m_type(type)
{
}

SceneNode::~SceneNode()
{
    clearChildren();
}

void
SceneNode::clearChildren()
{
    for ( int i = 0; i < m_children.size(); ++i )
        SAFE_DELETE( m_children[i] );
    m_children.clear();
}

void
SceneNode::addChild( SceneNode *child )
{
    m_children += child;
    child->m_parent = this;
    child->setCTMDirty();
}

void
SceneNode::deleteChild( SceneNode *child )
{
    int index = m_children.indexOf( child );
    if ( index != -1 ) {
        SceneNode *child = m_children[index];
        SAFE_DELETE( child );
        m_children.removeAt( index );
    }
}

void
SceneNode::setRenderable( shared_ptr<Renderable> spRenderable )
{
    m_spRenderable = spRenderable;// the reference count of spRenderable is
    //spRenderable.reset();
}


//void
//SceneNode::renderOpaque()
//{
//    glMatrixMode( GL_MODELVIEW );
//    glPushMatrix();
//    glMultMatrixf( glm::value_ptr(getCTM()) );
//    if ( m_spRenderable && !isTransparent() ) 
//        m_spRenderable->render();
//    glPopMatrix();
//    for ( int i = 0; i < m_children.size(); ++i )
//        m_children[i]->renderOpaque();
//}


void
SceneNode::renderMesh()
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixf( glm::value_ptr(getCTM()) );
    if ( m_spRenderable && isMesh() ) 
        m_spRenderable->render();
    glPopMatrix();
    for ( int i = 0; i < m_children.size(); ++i )
        m_children[i]->renderMesh();
}

void
SceneNode::renderParticleSystem()
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixf( glm::value_ptr(getCTM()) );
    if ( m_spRenderable && isParticleSystem() ) 
        m_spRenderable->render();
    glPopMatrix();
    for ( int i = 0; i < m_children.size(); ++i )
        m_children[i]->renderParticleSystem();
}

void
SceneNode::renderParticle()
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixf( glm::value_ptr(getCTM()) );
    if ( m_spRenderable && isParticle() ) 
        m_spRenderable->render();
    glPopMatrix();
    for ( int i = 0; i < m_children.size(); ++i )
        m_children[i]->renderParticle();
}

//void
//SceneNode::renderTransparent()
//{
//    glMatrixMode( GL_MODELVIEW );
//    glPushMatrix();
//    glMultMatrixf( glm::value_ptr(getCTM()) );
//    if ( m_spRenderable && isTransparent() ) m_spRenderable->render();
//    glPopMatrix();
//    for ( int i = 0; i < m_children.size(); ++i )
//        m_children[i]->renderTransparent();
//}


void
SceneNode::applyTransformation( const glm::mat4 &transform )
{
    m_transform = transform * m_transform;
    setCTMDirty();
    if(this->hasRenderable()) {
        getCTM();
        this->getRenderable()->setCTM(m_ctm);
    }
}
void
SceneNode::applyTransformationOnRendable( const glm::mat4 &transform )
{
    if(this->hasRenderable()) 
    {
        this->getRenderable()->applyTransformation( transform );
    }
}


glm::mat4
SceneNode::getCTM()
{
    if ( m_ctmDirty ) {
        glm::mat4 pCtm = ( m_parent ) ? m_parent->getCTM() : glm::mat4(1.0f);
        m_ctm = pCtm * m_transform;
        m_ctmDirty = false;
    }
    return m_ctm;
}
void
SceneNode::setCTMDirty()
{
    for ( int i = 0; i < m_children.size(); ++i ) {
        m_children[i]->setCTMDirty();
    }
    m_ctmDirty = true;
    m_centroidDirty = true;
}

glm::vec3
SceneNode::getCentroid()
{
    if ( m_centroidDirty ) {
        if ( hasRenderable() ) {
            m_centroid = m_spRenderable->getCentroid( getCTM() );
        } else {
            glm::vec4 p = getCTM() * glm::vec4(0,0,0,1);
            m_centroid = glm::vec3( p.x, p.y, p.z );
        }
        m_centroidDirty = false;
    }
    return m_centroid;
}
