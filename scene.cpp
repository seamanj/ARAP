//#include <Windows.h>
#include "particlesystem.h"
#include "scene.h"
#include "scenenode.h"
#include "common.h"
#include <GL/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include "mesh.h"
#include "objparser.h"
#include <memory>
#include "uisettings.h"
#include "particle.h"



using std::shared_ptr;
using glm::value_ptr;
using std::dynamic_pointer_cast;
Scene::Scene()
    : m_root(new SceneNode(SceneNode::Root))
{

}

Scene::~Scene()
{
    SAFE_DELETE( m_root );

}

void
Scene::reset()
{
    SAFE_DELETE(m_root);
    m_root = new SceneNode(SceneNode::Root);
}


void
Scene::render()
{
    setupLights();
    // Render opaque objects, then overlay with transparent objects
    //m_root->renderOpaque();
    //m_root->renderTransparent();
    if(UiSettings::showMesh())
        m_root->renderMesh();
    if(UiSettings::showPariticleSystem())
        m_root->renderParticleSystem();
    if(UiSettings::showParticle())
        m_root->renderParticle();
}

void
Scene::deleteSelectedNodes()
{
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->isSelectable() && node->getRenderable()->isSelected() ) {
            // Delete node through its parent so that the scene graph is appropriately
            // rid of the deleted node.
            node->parent()->deleteChild( node );
        } else {
            nodes += node->getChildren();
        }
    }
}
void
Scene::clearSelection()
{
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() )
    {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->getRenderable()->isSelected() )
        {
               node->getRenderable()->setSelected(false);
        }
        else
        {
                    nodes += node->getChildren();
        }
    }

}
void
Scene::deleteSelectedMeshes()
{
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->isMesh() && node->getRenderable()->isSelected() ) {
            // Delete node through its parent so that the scene graph is appropriately
            // rid of the deleted node.
            node->parent()->deleteChild( node );
        } else {
            nodes += node->getChildren();
        }
    }
}

void
Scene::roiSelectedParticles()
{
    ParticleSystem* ps = NULL;
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->isParticle() && node->getRenderable()->isSelected() ) {
            if( ps == NULL)
            {
                ps = dynamic_pointer_cast<Particle>(node->getRenderable())->getParent();
            }
            dynamic_pointer_cast<Particle>(node->getRenderable())->setROI(true);
            if( ps )
            {
                ps->insert_roi_vertex(dynamic_pointer_cast<Particle>(node->getRenderable())->m_order);
            }
        } else {
            nodes += node->getChildren();
        }
    }
}
void
Scene::deroiSelectedParticles()
{
    bool need_erase_control = false;
    ParticleSystem* ps = NULL;
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->isParticle() && node->getRenderable()->isSelected() )
        {

            if ( ps == NULL)
            {
                 ps = dynamic_pointer_cast<Particle>(node->getRenderable())->getParent();
            }
            if( !need_erase_control && dynamic_pointer_cast<Particle>(node->getRenderable())->isControl())
            {
                need_erase_control = true;
            }
            dynamic_pointer_cast<Particle>(node->getRenderable())->setROI(false);
            dynamic_pointer_cast<Particle>(node->getRenderable())->setControl(false);
            if( ps )
            {
                ps->erase_roi_vertex(dynamic_pointer_cast<Particle>(node->getRenderable())->m_order);
            }
        } else {
            nodes += node->getChildren();
        }
    }
    emit updateTool();
    if( ps && need_erase_control)
    {
        if(ps->curGroup != -1)
        {
            ps->getGroups().removeAt(ps->curGroup);
            if( ps->curGroup == ps->getGroups().size() - 1)
            {
                ps->curGroup = ps->curGroup - 1;

            }
        }
        emit updateTool();

    }

}

void
Scene::controlSelectedParticles()
{
    ParticleSystem* ps = NULL;
    QList<int> group;
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->isParticle() && node->getRenderable()->isSelected() ) {
            if( !dynamic_pointer_cast<Particle>(node->getRenderable())->isControl())
            {
                if( ps == NULL)
                {
                    ps = dynamic_pointer_cast<Particle>(node->getRenderable())->getParent();

                }
                group.push_back(dynamic_pointer_cast<Particle>(node->getRenderable())->m_order);
                dynamic_pointer_cast<Particle>(node->getRenderable())->setROI(true);
                dynamic_pointer_cast<Particle>(node->getRenderable())->setControl(true);
                if( ps )
                {
                    ps->insert_control_vertex(dynamic_pointer_cast<Particle>(node->getRenderable())->m_order);
                }
            }

        } else {
            nodes += node->getChildren();
        }
    }
    if( group.size())
    {
        ps->getGroups().push_back(group);
        ps->curGroup = ps->getGroups().size() - 1;
        emit updateTool();
    }
}
void
Scene::decontrolSelectedParticles()
{
    ParticleSystem* ps = NULL;
    QQueue<SceneNode*> nodes;
    nodes += m_root;

    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->hasRenderable() && node->isParticle() && node->getRenderable()->isSelected() )
        {
            if( !ps )
            {
                ps = dynamic_pointer_cast<Particle>(node->getRenderable())->getParent();
            }
            dynamic_pointer_cast<Particle>(node->getRenderable())->setControl(false);
            if( ps )
            {
                ps->erase_control_vertex(dynamic_pointer_cast<Particle>(node->getRenderable())->m_order);
            }
        } else {
            nodes += node->getChildren();
        }
    }

    if( ps )
    {
        if(ps->curGroup != -1)
        {
            ps->getGroups().removeAt(ps->curGroup);
            if( ps->curGroup == ps->getGroups().size() - 1)
            {
                ps->curGroup = ps->curGroup - 1;

            }
        }
        emit updateTool();

    }

}
void
Scene::selectPriorGroup()
{
    clearSelection();
    ParticleSystem* ps = NULL;
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->isParticleSystem())
        {
            ps = dynamic_pointer_cast<ParticleSystem>(node->getRenderable()).get();
        } else {
            nodes += node->getChildren();
        }
    }
    if( ps )
    {
        if( ps->curGroup != -1 )
            ps->curGroup = std::max(0, ps->curGroup - 1);
        else
        {
            if( ps->getGroups().size() )
                ps->curGroup = 0;
        }
        if( ps->curGroup != -1)
        {
            clearSelection();
            QMap<int, shared_ptr<Particle>>& particles = ps->getParticles();
            QList<int>& group = ps->m_groups[ps->curGroup];
            int size = group.size();
            for(int j = 0; j < size; ++j)
            {
                particles[group[j]]->setSelected(true);
            }


        }

    }
}
void
Scene::selectNextGroup()
{
    clearSelection();
    ParticleSystem* ps = NULL;
    QQueue<SceneNode*> nodes;
    nodes += m_root;
    while ( !nodes.empty() ) {
        SceneNode *node = nodes.dequeue();
        if ( node->isParticleSystem())
        {
            ps = dynamic_pointer_cast<ParticleSystem>(node->getRenderable()).get();
        } else {
            nodes += node->getChildren();
        }
    }
    if( ps )
    {
        int groupSize = ps->getGroups().size();

        if( ps->curGroup != -1 )
            ps->curGroup = std::min(groupSize - 1, ps->curGroup + 1);
        else
        {
            if( ps->getGroups().size() )
                ps->curGroup = 0;
        }
        if( ps->curGroup != -1)
        {
            clearSelection();
            QMap<int, shared_ptr<Particle>>& particles = ps->getParticles();
            QList<int>& group = ps->m_groups[ps->curGroup];
            int size = group.size();
            for(int j = 0; j < size; ++j)
            {
                particles[group[j]]->setSelected(true);
            }


        }

    }
}
void
Scene::setupLights()
{
//    glm::vec4 diffuse = glm::vec4( 0.5f, 0.5f, 0.5f, 1.f );
//    for ( int i = 0; i < 5; ++i ) {
//        glEnable( GL_LIGHT0 + i );
//        glLightfv( GL_LIGHT0 + i, GL_DIFFUSE, glm::value_ptr(diffuse) );
//    }

//    glLightfv( GL_LIGHT0, GL_POSITION, glm::value_ptr(glm::vec4(100.f, 0.f, 0.f, 1.f)) );
//    glLightfv( GL_LIGHT1, GL_POSITION, glm::value_ptr(glm::vec4(-100.f, 0.f, 0.f, 1.f)) );
//    glLightfv( GL_LIGHT2, GL_POSITION, glm::value_ptr(glm::vec4(0.f, 0.f, 100.f, 1.f)) );
//    glLightfv( GL_LIGHT3, GL_POSITION, glm::value_ptr(glm::vec4(0.f, 0.f, -100.f, 1.f)) );
//    glLightfv( GL_LIGHT4, GL_POSITION, glm::value_ptr(glm::vec4(0.f, 100.f, 0.f, 1.f)) );
    glm::vec4 diffuse = glm::vec4( 0.8f, 0.8f, 0.8f, 1.f );

    glEnable( GL_LIGHT0 );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(diffuse) );
    glLightfv( GL_LIGHT0, GL_POSITION, glm::value_ptr(glm::vec4(0.f, 100.f, 100.f, 1.f)) );

}

SceneNodeIterator
Scene::begin() const
{
    QList<SceneNode*> nodes;
    nodes += m_root;
    int i = 0;
    while ( i < nodes.size() ) {
        nodes += nodes[i]->getChildren();
        i++;
    }
    return SceneNodeIterator( nodes );
}


void
Scene::loadMesh(const QString &filename, glm::mat4 CTM)
{
    QList<shared_ptr<Mesh>> spMeshes;
    OBJParser::load( filename, spMeshes );
    for ( int i = 0; i < spMeshes.size(); ++i ) {
        shared_ptr<Mesh> spMesh = spMeshes[i];
        SceneNode *node = new SceneNode( SceneNode::Mesh );
        node->setRenderable( spMesh );
        node->applyTransformation(CTM);
        m_root->addChild(node);
    }
}


