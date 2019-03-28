#ifndef SCENENODE_H
#define SCENENODE_H

#include <QList>
#include "renderable.h"
#include <glm/vec3.hpp>
#include <memory>
#include "particle.h"
using std::shared_ptr;
using std::dynamic_pointer_cast;
class Renderable;

class SceneNode
{
public:

    enum Type
    {
        Root,
        Mesh,
        ParticleSystem,
        Particle
    };

    SceneNode( Type type );
    virtual ~SceneNode();

    void clearChildren();
    void addChild( SceneNode *child );
    void deleteChild( SceneNode *child );
    SceneNode* parent() { return m_parent; }
    QList<SceneNode*> getChildren() { return m_children; }

    bool hasRenderable() const { return m_spRenderable != NULL; }
    void setRenderable( shared_ptr<Renderable> spRenderable );
    shared_ptr<Renderable>& getRenderable() { return m_spRenderable; }

    // Render the node's renderable if it is opaque
    //virtual void renderOpaque();
    virtual void renderMesh();
    virtual void renderParticleSystem();
    virtual void renderParticle();
    // Render the node's renderable if it is transparent
    //virtual void renderTransparent();
    void setCTMDirty();
    glm::mat4 getCTM();
    bool m_ctmDirty;
    void applyTransformation( const glm::mat4 &transform );
    void applyTransformationOnRendable( const glm::mat4 &transform );
    Type getType() { return m_type; }

    //bool isTransparent() const { return m_type == Transparent; }
    bool isMesh() const {return m_type == Mesh;}
    bool isParticleSystem() const {return m_type == ParticleSystem;}
    bool isParticle() const {return m_type == Particle;}
    bool isSelectable() const
    {
        if(m_type == Mesh)
            return true;
        if( m_type == Particle)
        {
            if(!dynamic_pointer_cast<::Particle>(m_spRenderable)->isControl())
                return true;
        }
        return false;
    }
    bool isMovable() const
    {
        if(m_type == Mesh)
            return true;
        if( m_type == Particle)
        {
            if(dynamic_pointer_cast<::Particle>(m_spRenderable)->isControl())
                return true;
        }
        return false;
    }
    bool isRotatable() const {return m_type == Mesh;}
    bool isScable() const {return m_type == Mesh;}
    glm::vec3 getCentroid();
private:

    SceneNode* m_parent;

    // The following member variables depend on the scene node's
    // cumulative transformation, so they are cached and only
    // recomputed when necessary, if they are labeled "dirty".
    glm::mat4 m_ctm;

    glm::vec3 m_centroid;
    bool m_centroidDirty;

    glm::mat4 m_transform;

    QList<SceneNode*> m_children;
    shared_ptr<Renderable> m_spRenderable;

    Type m_type;

};

#endif // SCENENODE_H
