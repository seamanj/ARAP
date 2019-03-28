#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "common.h"
class SceneNode;
class Renderable
{
public:
    Renderable() : m_selected(false){}
    virtual ~Renderable(){}

    virtual void render() = 0;
    virtual void renderForPicker() = 0;

    virtual void setCTM(const glm::mat4 &ctm){m_ctm = ctm;}
    virtual void applyTransformation( const glm::mat4 &transform )
    {
        LOG( "Warning - applyTransformation : reach <Renderable> base class virtual function." );
    }
    virtual void setSelected( bool selected ) { m_selected = selected; }
    bool isSelected() const { return m_selected; }

    
    virtual glm::vec3 getCentroid( const glm::mat4 &ctm = glm::mat4(1.f) )
    {
        LOG( "Warning - getCentroid : reach <Renderable> base class virtual function." );
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    
    inline bool hasSceneNode(){return m_sceneNode != NULL;}
    inline void setSceneNode(SceneNode* sceneNode){m_sceneNode = sceneNode;}
    inline SceneNode* getSceneNode(){return m_sceneNode;} 

protected:
    glm::mat4 m_ctm;
    bool m_selected;
    SceneNode* m_sceneNode;
};

#endif // RENDERABLE_H
