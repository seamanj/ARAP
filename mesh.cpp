#include "mesh.h"
//#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include "uisettings.h"


Mesh::Mesh()
    : m_glVBO(0),
      m_color(0.4f, 0.4f, 0.4f, 1.f)
{

}
Mesh::~Mesh()
{
    deleteVBO();
}

void
Mesh::computeNormals()
{
    Normal *triNormals = new Normal[getNumTris()];
    float *triAreas = new float[getNumTris()];
    QVector<int> *vertexMembership = new QVector<int>[getNumVertices()];
    for ( int i = 0; i < getNumTris(); ++i ) {
        // Compute triangle normal and area
        const Tri &tri = m_tris[i];
        const Vertex &v0 = m_vertices[tri[0]];
        const Vertex &v1 = m_vertices[tri[1]];
        const Vertex &v2 = m_vertices[tri[2]];
        Normal n = glm::cross(v1-v0, v2-v0);
        triAreas[i] = n.length()/2.f;
        triNormals[i] = 2.f*n/triAreas[i];
        // Record triangle membership for each vertex
        vertexMembership[tri[0]] += i;
        vertexMembership[tri[1]] += i;
        vertexMembership[tri[2]] += i;
    }

    m_normals.clear();
    m_normals.resize( getNumVertices() );
    for ( int i = 0; i < getNumVertices(); ++i ) {
        Normal normal = Normal( 0.f, 0.f, 0.f );
        float sum = 0.f;
        for ( int j = 0; j < vertexMembership[i].size(); ++j ) {
            int index = vertexMembership[i][j];
            normal += triAreas[index]*triNormals[index];
            sum += triAreas[index];
        }
        normal /= sum;
        m_normals[i] = normal;
    }

    delete [] triNormals;
    delete [] triAreas;
    delete [] vertexMembership;
}
bool
Mesh::hasVBO() const
{
    bool has = false;
    if ( m_glVBO > 0 ) {
        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO );
        has = glIsBuffer( m_glVBO );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }
    return has;
}

void
Mesh::deleteVBO()
{
    if ( m_glVBO > 0 ) {
        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO );
        if ( glIsBuffer(m_glVBO) ) {
            glDeleteBuffers( 1, &m_glVBO );
        }
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        m_glVBO = 0;
    }
}
void
Mesh::buildVBO()
{
    deleteVBO();

    // Create flat array of non-indexed triangles
    glm::vec3 *data = new glm::vec3[6*getNumTris()];
    for ( int i = 0, index = 0; i < getNumTris(); ++i ) {
        const Tri &tri = m_tris[i];
        data[index++] = m_vertices[tri[0]];
        data[index++] = m_normals[tri[0]];
        data[index++] = m_vertices[tri[1]];
        data[index++] = m_normals[tri[1]];
        data[index++] = m_vertices[tri[2]];
        data[index++] = m_normals[tri[2]];
    }

    // Build OpenGL VBO
    glGenBuffers( 1, &m_glVBO );
    glBindBuffer( GL_ARRAY_BUFFER, m_glVBO );
    glBufferData( GL_ARRAY_BUFFER, 6*getNumTris()*sizeof(glm::vec3), data, GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );



    delete [] data;
}
void
Mesh::renderVBO()
{
    glBindBuffer( GL_ARRAY_BUFFER, m_glVBO );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 2*sizeof(glm::vec3), (void*)(0) );
    glEnableClientState( GL_NORMAL_ARRAY );
    glNormalPointer( GL_FLOAT, 2*sizeof(glm::vec3), (void*)(sizeof(glm::vec3)) );

    glDrawArrays( GL_TRIANGLES, 0, 3*getNumTris() );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
}

void
Mesh::render()
{

    if ( !hasVBO() ) {
        buildVBO();
    }

    glPushAttrib( GL_DEPTH_TEST );
    glEnable( GL_DEPTH_TEST );

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    glPushAttrib( GL_COLOR_BUFFER_BIT );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glm::vec4 color = ( m_selected ) ? glm::mix( m_color, UiSettings::meshSelectionColor(), 0.5f ) : m_color;

   
        
    glPushAttrib( GL_POLYGON_BIT );
    glPushAttrib( GL_LIGHTING_BIT );
    glEnable( GL_LIGHTING );
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(color*0.2f) );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(color) );
     if( UiSettings::showMeshesMode() == UiSettings::SOLID )
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
     else if(UiSettings::showMeshesMode() == UiSettings::WIREFRAME) 
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
     else if(UiSettings::showMeshesMode() == UiSettings::POINT)
     {
        
         glEnable( GL_POINT_SMOOTH );
         glPointSize(10.0f);
         glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
     }
    renderVBO();
    glPopAttrib();
    glPopAttrib();
    
    glPopAttrib();
    glPopAttrib();

}
void
Mesh::renderForPicker()
{

    if ( !hasVBO() ) {
        buildVBO();
    }

    glPushAttrib( GL_DEPTH_TEST );
    glEnable( GL_DEPTH_TEST );
    glPushAttrib( GL_LIGHTING_BIT );
    glDisable( GL_LIGHTING );
    glColor3f( 1.f, 1.f, 1.f );
    renderVBO();
    glPopAttrib();
    glPopAttrib();
}
glm::vec3
Mesh::getCentroid( const glm::mat4 &ctm )
{
    glm::vec3 c(0,0,0);
    for ( int i = 0; i < getNumVertices(); ++i ) {
        const Vertex &v = m_vertices[i];
        glm::vec4 point = ctm * glm::vec4( glm::vec3(v), 1.f );
        c += glm::vec3( point.x, point.y, point.z );
    }
    return c / (float)getNumVertices();
}

