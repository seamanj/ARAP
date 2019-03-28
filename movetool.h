#ifndef MOVETOOL_H
#define MOVETOOL_H

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include "glm/vec2.hpp"

#include "selectiontool.h"

typedef unsigned int GLuint;

class MoveTool : public SelectionTool
{
    
public:
    
    MoveTool( ViewPanel *panel,Type t );
    virtual ~MoveTool();
    
    virtual void mousePressed();
    virtual void mouseMoved();
    virtual void mouseReleased();
    
    //virtual void update();
    
    virtual void render();
public slots:
    virtual void update();
protected:
    
    unsigned int m_axisSelection;
    
    bool m_active;
    bool m_moving;
    glm::vec3 m_center;
    float m_scale;
    
    GLuint m_vbo;
    int m_vboSize;
    
    void renderAxis( unsigned int i ) const;
    void renderCenter() const;
    unsigned int getAxisPick() const;
    float intersectAxis( const glm::ivec2 &mouse ) const;
    
    bool hasVBO() const;
    void buildVBO();
    void deleteVBO();

    int count;
    
};

#endif // MOVETOOL_H
