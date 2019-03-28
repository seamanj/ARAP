#ifndef PARTICLE_H
#define PARTICLE_H

#include "renderable.h"
#include "types.h"
class ParticleSystem;
class Particle : public Renderable
{
    friend class ParticleSystem;
public:
    Particle();
    Particle(int order, Vertex position, Normal normal, ParticleSystem* parent, bool ROI = false, bool control = false);
    ~Particle();
    virtual void render();
    virtual void renderForPicker();
    virtual glm::vec3 getCentroid( const glm::mat4 &ctm = glm::mat4(1.f) );
    inline void setROI(bool ROI){ m_ROI = ROI;}
    inline void setControl(bool control) { m_control = control;}
    inline bool isROI(){return m_ROI;}
    inline bool isControl(){return m_control;}
    virtual void applyTransformation( const glm::mat4 &transform );
    inline ParticleSystem* getParent(){ return m_parent;}
protected:
    // position
    Vertex m_position;
    // normal
    Normal m_normal;
    int m_order;
    bool m_ROI;
    bool m_control;
    ParticleSystem* m_parent;
    friend class Engine;
    friend class SelectionTool;
    friend class Scene;
};

#endif // PARTICLE_H
