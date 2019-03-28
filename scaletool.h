#ifndef SCALETOOL_H
#define SCALETOOL_H

#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "selectiontool.h"


typedef unsigned int GLuint;

class ScaleTool : public SelectionTool
{

public:

    ScaleTool( ViewPanel *panel,Type t );
    virtual ~ScaleTool();

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
    bool m_scaling;
    glm::vec3 m_center;
    float m_scale;

    glm::ivec2 m_mouseDownPos;
    glm::mat4 m_transformInverse;
    glm::mat4 m_transform;

    GLuint  m_vbo;
    int m_vboSize;
    float m_radius;

    void renderAxis( unsigned int i ) const;
    void renderCenter() const;

    unsigned int getAxisPick() const;
    float intersectAxis( const glm::ivec2 &mouse ) const;

    bool hasVBO() const;
    void buildVBO();
    void deleteVBO();

};

#endif // SCALETOOL_H
