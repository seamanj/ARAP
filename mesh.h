#ifndef MESH_H
#define MESH_H

#include "renderable.h"
#include "types.h"
#include "scenenode.h"
#include <QString>
#include <QVector>

typedef unsigned int GLuint;
class SceneNode;
class ParticleSystem;
class Mesh : public Renderable
{
    friend class ParticleSystem;
public:
    Mesh();
    virtual ~Mesh();


    void deleteVBO();
    inline void setName( const QString &name ) { m_name = name; }
    inline QString getName() const { return m_name; }

    inline void setFilename( const QString &filename ) { m_filename = filename; }
    inline QString getFilename() const { return m_filename; }

    inline Vertex& getVertex( int i ) { return m_vertices[i]; }
    inline Vertex getVertex( int i ) const { return m_vertices[i]; }

    inline Tri& getTri( int i ) { return m_tris[i]; }
    inline Tri getTri( int i ) const { return m_tris[i]; }

    inline int getNumVertices() const { return m_vertices.size(); }
    inline int getNumNormals() const { return m_normals.size(); }
    inline int getNumTris() const { return m_tris.size(); }
    void computeNormals();
    inline void setVertices( const QVector<Vertex> &vertices ) { m_vertices = vertices; }
    inline void setTris( const QVector<Tri> &tris ) { m_tris = tris; }
    inline void setNormals( const QVector<Normal> &normals ) { m_normals = normals; }
    virtual void render();
    virtual void renderForPicker();
    bool hasVBO() const;
    void buildVBO();
    ParticleSystem* particlize();

    void renderVBO();
    virtual glm::vec3 getCentroid( const glm::mat4 &ctm = glm::mat4(1.f) );
    

    
private:
    QString m_name;
    QString m_filename; // The OBJ file source
    GLuint m_glVBO;
    Color m_color;

    // List of vertices
    QVector<Vertex> m_vertices;

    // List of tris, which index into vertices
    QVector<Tri> m_tris;

    // List of vertex normals
    QVector<Normal> m_normals;
    SceneNode* m_sceneNode;
};

#endif // MESH_H
