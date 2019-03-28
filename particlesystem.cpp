#include <GL/glew.h>
#include <GL/gl.h>
#include "particlesystem.h"
//#include <Windows.h>

#include "types.h"
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "uisettings.h"
#include <vector>
#include <CGAL/config.h>
using std::vector;
ParticleSystem::ParticleSystem(Type type) : m_type(type), m_color(88.f / 255, 45.f / 255, 216.f /255, 1.0f), m_verticesNum(0), m_triNum(0),
    curGroup(-1)
{
    m_glVBO[0] = 0;
    m_glVBO[1] = 0;
}
void ParticleSystem::initFromPolyhedron(shared_ptr<MyPolyhedron<Polyhedron>> spMyPolyhedron)
{
    typedef  boost::property_map<Polyhedron, CGAL::vertex_point_t>::type Vertex_point_map;
    typedef  MyPolyhedron<Polyhedron>::GraphTraits GraphTraits;
    typedef  GraphTraits::face_iterator face_iterator;
    typedef  GraphTraits::edge_iterator edge_iterator;
    typedef  GraphTraits::vertex_iterator vertex_iterator;
    typedef  GraphTraits::halfedge_descriptor halfedge_descriptor;
    typedef  GraphTraits::vertex_descriptor vertex_descriptor;
    typedef  boost::property_traits<Vertex_point_map>::value_type Point;

    setMyPolyhedron(spMyPolyhedron);

    glm::mat4 ctm(1.f);
    if( SceneNode* sceneNode = spMyPolyhedron->getSceneNode() )
         ctm = sceneNode->getCTM();
    vertex_iterator vb, ve;
    for(boost::tie(vb, ve) = vertices(*spMyPolyhedron->m_halfedge_graph); vb != ve; ++vb)
    {
        Point& p = spMyPolyhedron->vertex_point_map[*vb];
        glm::vec4 point = ctm * glm::vec4(p.x(), p.y(), p.z(), 1.f );
        p = Point(point.x, point.y, point.z);
        Vector& n = (*vb)->normal();
        glm::vec4 normal = glm::transpose(glm::inverse(ctm)) * glm::vec4(n.x(), n.y(), n.z(), 0.f);
        n = Vector(normal.x, normal.y, normal.z);
        shared_ptr<Particle> spParticle( new Particle((*vb)->id(), glm::vec3( point.x, point.y, point.z ), glm::vec3( normal.x, normal.y, normal.z ), this));
        m_particles.insert((*vb)->id(), spParticle);
    }
}
void ParticleSystem::initFromMesh(shared_ptr<Mesh> spMesh)
{
    QVector<Vertex> vertices(spMesh->m_vertices);
    QVector<Normal> normals(spMesh->m_normals);
    
    m_tris =spMesh->m_tris;
    glm::mat4 ctm(1.f);
    if( SceneNode* sceneNode = spMesh->getSceneNode() )
         ctm = sceneNode->getCTM();
    for(int i = 0; i < vertices.size(); ++i)
    {
        glm::vec4 point = ctm * glm::vec4(vertices[i], 1.f );
        glm::vec4 normal = glm::transpose(glm::inverse(ctm)) * glm::vec4(normals[i],0.f);
        shared_ptr<Particle> spParticle( new Particle(i, glm::vec3( point.x, point.y, point.z ), glm::vec3( normal.x, normal.y, normal.z ), this));
        m_particles.insert(i, spParticle);
    }
    m_verticesNum = m_particles.size();
    m_triNum = m_tris.size();
    
    m_currentPositions.resize( m_verticesNum * 3 );
    m_originalPositions.resize( m_verticesNum * 3 );
    m_previousPositions.resize( m_verticesNum * 3 );
    m_currentVelocities.resize( m_verticesNum * 3);
    m_previousVelocities.resize( m_verticesNum * 3);
    m_externalForce.resize( m_verticesNum * 3);
    
    m_massMatrix.resize( m_verticesNum * 3, m_verticesNum * 3);
    m_invMassMatrix.resize( m_verticesNum * 3, m_verticesNum * 3);
    
    
    for(int i=0; i< m_verticesNum; ++i)
    {
        Eigen::Map<EigenVector3> v(glm::value_ptr(m_particles[i]->m_position), 3);
        m_currentPositions.block_vector(i) = v;
        m_originalPositions.block_vector(i)= v;
        m_previousPositions.block_vector(i) = v;
        m_currentVelocities.block_vector(i) = EigenVector3(0, 0, 0);
        m_previousVelocities.block_vector(i) = EigenVector3(0, 0, 0);
        m_externalForce.block_vector(i)= EigenVector3(0, -0.98, 0);
    }
    
    vector<SparseMatrixTriplet> massTriplets;
    vector<SparseMatrixTriplet> massInvTriplets;
    
    ScalarType unitMass = 1;
    ScalarType massInv = 1.0 / unitMass;
    for (int index = 0; index < m_verticesNum * 3; index++)
    {
        massTriplets.push_back(SparseMatrixTriplet(index, index, unitMass));
        massInvTriplets.push_back(SparseMatrixTriplet(index, index, massInv));
    }
    
    m_massMatrix.setFromTriplets(massTriplets.begin(), massTriplets.end());
    m_invMassMatrix.setFromTriplets(massInvTriplets.begin(), massInvTriplets.end());
    
    
    m_q.resize( m_verticesNum * 3);
    m_qTilda.resize( m_verticesNum * 9);
    m_p.resize( m_verticesNum * 3);
    m_g.resize( m_verticesNum * 3);

    
}


void ParticleSystem::render()
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

    glm::vec4 color = ( m_selected ) ? glm::mix( m_color, UiSettings::particleSystemSelectionColor(), 0.1f ) : m_color;
    //glm::vec4 color = glm::mix( m_color, UiSettings::particleSystemSelectionColor(), 0.5f );
   
        
    glPushAttrib( GL_POLYGON_BIT );
    glPushAttrib( GL_LIGHTING_BIT );
    glEnable( GL_LIGHTING );
    //glm::vec4 lightColor =glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(color));
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(color) );
     if( UiSettings::showParticleSystemsMode() == UiSettings::SOLID )
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
     else if(UiSettings::showParticleSystemsMode() == UiSettings::WIREFRAME) 
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
//     else if(UiSettings::showParticleSystemsMode() == UiSettings::POINT)
//     {
//         glEnable( GL_POINT_SMOOTH );
//         glPointSize(10.0f);
//         glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
//     }
    renderVBO();
    glPopAttrib();
    glPopAttrib();
    
    glPopAttrib();
    glPopAttrib();
}
void
ParticleSystem::renderForPicker()
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

bool
ParticleSystem::hasVBO() const
{
    bool has = false;
    if ( m_glVBO[0] > 0 && m_glVBO[1] > 0 ) {
        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[0] );
        has |= glIsBuffer( m_glVBO[0] );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[1] );
        has |= glIsBuffer( m_glVBO[1] );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }
    return has;
}
void
ParticleSystem::buildVBO()
{
    deleteVBO();

    typedef boost::graph_traits<Polyhedron> GraphTraits;
    typedef GraphTraits::face_iterator face_iterator;
    typedef GraphTraits::edge_iterator edge_iterator;
    typedef GraphTraits::halfedge_descriptor halfedge_descriptor;


    glm::vec3 *data = new glm::vec3[6*num_faces(*(m_spMyPolyhedron->m_halfedge_graph))];
    glm::vec3 *edgeData = new glm::vec3[2*num_edges(*(m_spMyPolyhedron->m_halfedge_graph))];
    int index = 0, edgeIndex = 0;

    face_iterator fb, fe;
    for(boost::tie(fb, fe) = faces(*(m_spMyPolyhedron->m_halfedge_graph)); fb != fe; ++fb)
      {
        halfedge_descriptor edg = halfedge(*fb, *(m_spMyPolyhedron->m_halfedge_graph));
        halfedge_descriptor edgb = edg;

        Point p0 = m_spMyPolyhedron->vertex_point_map[target(edg, *(m_spMyPolyhedron->m_halfedge_graph))];

        Vector n0 = target(edg, *(m_spMyPolyhedron->m_halfedge_graph))->normal();
        edg = next(edg, *(m_spMyPolyhedron->m_halfedge_graph));


        Point p1 = m_spMyPolyhedron->vertex_point_map[target(edg, *(m_spMyPolyhedron->m_halfedge_graph))];
        Vector n1 = target(edg, *(m_spMyPolyhedron->m_halfedge_graph))->normal();
        edg = next(edg, *(m_spMyPolyhedron->m_halfedge_graph));



        Point p2 = m_spMyPolyhedron->vertex_point_map[target(edg, *(m_spMyPolyhedron->m_halfedge_graph))];
        Vector n2 = target(edg, *(m_spMyPolyhedron->m_halfedge_graph))->normal();
        edg = next(edg, *(m_spMyPolyhedron->m_halfedge_graph));

        //cout << "p0: " << p0 << " p1: " << p1 << " p2: " << p2 << endl;
        //cout << "n0: " << n0 << " n1: " << n1 << " n2: " << n2 << endl;
        if(edg == edgb)
        {
          // triangle
              data[index++] = glm::vec3(p0.x(), p0.y(), p0.z());
              data[index++] = glm::vec3(n0.x(), n0.y(), n0.z());
              data[index++] = glm::vec3(p1.x(), p1.y(), p1.z());
              data[index++] = glm::vec3(n1.x(), n1.y(), n1.z());
              data[index++] = glm::vec3(p2.x(), p2.y(), p2.z());
              data[index++] = glm::vec3(n2.x(), n2.y(), n2.z());
        }

    }

    edge_iterator eb, ee;
    for(boost::tie(eb, ee) = edges(*(m_spMyPolyhedron->m_halfedge_graph)); eb != ee; ++eb)
    {
        Point p0 = m_spMyPolyhedron->vertex_point_map[source(*eb, *(m_spMyPolyhedron->m_halfedge_graph))];
        Point p1 = m_spMyPolyhedron->vertex_point_map[target(*eb, *(m_spMyPolyhedron->m_halfedge_graph))];
        edgeData[edgeIndex++] = glm::vec3(p0.x(), p0.y(), p0.z());
        edgeData[edgeIndex++] = glm::vec3(p1.x(), p1.y(), p1.z());

    }


    // Build OpenGL VBO
    glGenBuffers( 2, &m_glVBO[0] );
    glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[0] );
    glBufferData( GL_ARRAY_BUFFER, 6*num_faces(*(m_spMyPolyhedron->m_halfedge_graph))*sizeof(glm::vec3), data, GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[1] );
    glBufferData( GL_ARRAY_BUFFER, 2*num_edges(*(m_spMyPolyhedron->m_halfedge_graph))*sizeof(glm::vec3), edgeData, GL_STATIC_DRAW );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );



    delete [] data;
}
void
ParticleSystem::renderVBO()
{
    glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[0] );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, 2*sizeof(glm::vec3), (void*)(0) );
    glEnableClientState( GL_NORMAL_ARRAY );
    glNormalPointer( GL_FLOAT, 2*sizeof(glm::vec3), (void*)(sizeof(glm::vec3)) );

    glDrawArrays( GL_TRIANGLES, 0, 3 * num_faces(*(m_spMyPolyhedron->m_halfedge_graph)) );

    glDisableClientState( GL_NORMAL_ARRAY );


//    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(glm::vec4(0, 0, 0, 1)) );
//    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(glm::vec4(0, 0, 0, 1)) );

//    glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[1] );
//    glEnableClientState( GL_VERTEX_ARRAY );
//    glVertexPointer( 3, GL_FLOAT, 0, (void*)(0) );

//    glDrawArrays( GL_LINES, 0, 2 * num_edges(*(m_spMyPolyhedron->m_halfedge_graph)) );

//    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableClientState( GL_VERTEX_ARRAY );
}

void
ParticleSystem::deleteVBO()
{
    for(int i = 0; i < 2; ++i )
    {
        if ( m_glVBO[i] > 0 ) {
            glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[i] );
            if ( glIsBuffer(m_glVBO[i]) ) {
                glDeleteBuffers( 1, &m_glVBO[i] );
            }
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            m_glVBO[i] = 0;
        }

    }
}
glm::vec3
ParticleSystem::getCentroid( const glm::mat4 &ctm )
{
    glm::vec3 c(0,0,0);
    int numVertices = m_particles.size();
    for ( int i = 0; i < numVertices; ++i ) {
        const Vertex &v = m_particles[i]->m_position;
        glm::vec4 point = ctm * glm::vec4( glm::vec3(v), 1.f );
        c += glm::vec3( point.x, point.y, point.z );
    }
    return c / (float)numVertices;
}
void
ParticleSystem::update()//update particles from engine
{
    deleteVBO();
    for(int i = 0; i < m_verticesNum; ++i)
    {
        EigenVector3 v = m_currentPositions.block_vector(i);
        m_particles[i]->m_position = glm::vec3(v.x(), v.y(), v.z());
        
    }
}
bool
ParticleSystem::insert_roi_vertex(int i)
{
    return m_spMyPolyhedron->insert_roi_vertex(i);
}

bool
ParticleSystem::erase_roi_vertex(int i)
{
    return m_spMyPolyhedron->erase_roi_vertex(i);
}

bool
ParticleSystem::insert_control_vertex(int i)
{
    return m_spMyPolyhedron->insert_control_vertex(i);
}
bool
ParticleSystem::erase_control_vertex(int i)
{
    return m_spMyPolyhedron->erase_control_vertex(i);
}

void
ParticleSystem::updateFromParticles()//update m_currentPositions from engine
{
    deleteVBO();
    for(int i = 0; i < m_verticesNum; ++i)
    {
        glm::vec3 v = m_particles[i]->m_position;
        m_currentPositions.block_vector(i) = EigenVector3(v.x, v.y, v.z);
        
    }
}

void ParticleSystem::set_target_vertex(int i, Vertex pos)
{
    return m_spMyPolyhedron->set_target_vertex(i, pos);
}

void ParticleSystem::deform()
{
    m_spMyPolyhedron->deform();
    deleteVBO();
    for(std::size_t i = 0; i < m_spMyPolyhedron->ros.size(); ++i)
    {
        std::size_t v_id = m_spMyPolyhedron->vd_to_id(m_spMyPolyhedron->ros[i]);
        std::size_t v_ros_id = m_spMyPolyhedron->vd_to_ros_id(m_spMyPolyhedron->ros[i]);
        MyPolyhedron<Polyhedron>::Point p = m_spMyPolyhedron->solution[v_ros_id];
        m_particles[v_id]->m_position = glm::vec3(p.x(), p.y(), p.z());

    }
}
