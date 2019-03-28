#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H
#include "mypolyhedron.h"
#include <QVector>
#include <QMap>
#include "common.h"
#include "renderable.h"
#include "particle.h"
#include "mesh.h"
#include <memory>
#include "types.h"
#include <CGAL/Polygon_mesh_processing/Weights.h>


using std::shared_ptr;
template<
    class HG
        >
class MyPolyhedron;

//typedef CGAL::Surface_mesh_deformation<Polyhedron> Surface_mesh_deformation;

class ParticleSystem : public Renderable
{
public:
    enum Type
    {
        Zero,
        Rigid,
        Linear,
        Quadratic
    };


    ParticleSystem(Type type = Rigid);
    void initFromMesh(shared_ptr<Mesh> spMesh);
    void initFromPolyhedron(shared_ptr<MyPolyhedron<Polyhedron>> spMyPolyhedron);
    void setMyPolyhedron(shared_ptr<MyPolyhedron<Polyhedron>>& spMyPolyhedron)
    {
        m_spMyPolyhedron = spMyPolyhedron;
    }
    virtual void render();
    virtual void renderForPicker();
    bool hasVBO() const;
    void buildVBO();
    void renderVBO();
    void deleteVBO();
    inline int getNumTris() const { return m_tris.size(); }
    inline QList<QList<int>>& getGroups() {return m_groups;}
    virtual glm::vec3 getCentroid( const glm::mat4 &ctm = glm::mat4(1.f) );
    QMap<int, shared_ptr<Particle>>& getParticles(){ return m_particles;}
    
    void update();
    void updateFromParticles();
    void setDeformationMode(int mode) {m_type = Type(mode);}
    Type& getDeformationMode(){ return m_type;}
    
    bool insert_roi_vertex(int i);
    bool erase_roi_vertex(int i);

    bool insert_control_vertex(int i);
    bool erase_control_vertex(int i);

    void set_target_vertex(int i, Vertex pos);
    void deform();
protected:
    QMap<int, shared_ptr<Particle>> m_particles;
    QVector<Tri> m_tris;
    GLuint m_glVBO[2];
    Color m_color;
    // Eigen Stuff
    
    VectorX m_currentPositions;
    VectorX m_originalPositions;
    VectorX m_previousPositions;
    VectorX m_currentVelocities;
    VectorX m_previousVelocities;
    VectorX m_externalForce;
    SparseMatrix m_massMatrix;
    SparseMatrix m_invMassMatrix;
    
    //Shape matching
    EigenVector3 m_t0;
    EigenVector3 m_t;
    VectorX m_q;
    VectorX m_qTilda;
    VectorX m_p;
    VectorX m_g;
    EigenMatrix3 m_Aqq;
    EigenMatrix9x9 m_AqqTilda;
    EigenMatrix3 m_Apq;
    EigenMatrix3x9 m_ApqTilda;
    EigenMatrix3 m_A;
    EigenMatrix3x9 m_ATilda;
    EigenMatrix3 m_R;
    EigenMatrix3x9 m_RTilda;
    
    unsigned int m_verticesNum;
    unsigned int m_triNum;
    
    Type m_type;
    shared_ptr<MyPolyhedron<Polyhedron>> m_spMyPolyhedron;
    QList<QList<int>> m_groups;
    int curGroup;


    friend class Engine;
    friend class Scene;
};

#endif // PARTICLESYSTEM_H
