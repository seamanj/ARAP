#pragma once
#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include "uisettings.h"

#include "types.h"
#include "renderable.h"
#include "graphfunction.h"
#include "particlesystem.h"

#include <CGAL/config.h>
#include <CGAL/Polygon_mesh_processing/Weights.h>
#include <CGAL/Deformation_Eigen_polar_closest_rotation_traits_3.h>
#include <CGAL/Eigen_solver_traits.h>
using std::shared_ptr;







template<
    class HG
        >
class MyPolyhedron : public Renderable
{
public:
    typedef HG Halfedge_graph;
    typedef typename boost::property_map<Halfedge_graph, boost::vertex_index_t>::type Vertex_index_map;
    typedef typename boost::property_map<Halfedge_graph, boost::halfedge_index_t>::type Hedge_index_map;
    typedef typename boost::property_map<Halfedge_graph, CGAL::vertex_point_t>::type Vertex_point_map;
    typedef boost::graph_traits<Halfedge_graph> GraphTraits;
    typedef typename GraphTraits::face_iterator face_iterator;
    typedef typename GraphTraits::edge_iterator edge_iterator;
    typedef typename GraphTraits::vertex_iterator vertex_iterator;
    typedef typename GraphTraits::vertex_descriptor vertex_descriptor;
    typedef typename GraphTraits::halfedge_descriptor halfedge_descriptor;
    typedef typename boost::property_traits<Vertex_point_map>::value_type Point;
    typedef typename CGAL::internal::Cotangent_weight<Halfedge_graph> Weight_calculator;
    typedef typename CGAL::Deformation_Eigen_polar_closest_rotation_traits_3 Closest_rotation_traits;
    typedef typename boost::graph_traits<Halfedge_graph>::halfedge_iterator       halfedge_iterator;
    typedef typename boost::graph_traits<Halfedge_graph>::in_edge_iterator    in_edge_iterator;
    typedef typename boost::graph_traits<Halfedge_graph>::out_edge_iterator   out_edge_iterator;

    typedef typename Closest_rotation_traits::Matrix CR_matrix;
    typedef typename Closest_rotation_traits::Vector CR_vector;


    typedef typename CGAL::Eigen_solver_traits<
    Eigen::SparseLU<
      CGAL::Eigen_sparse_matrix<double>::EigenType,
      Eigen::COLAMDOrdering<int> >  > Sparse_linear_solver;


    MyPolyhedron(Halfedge_graph* halfedge_graph)
        : vertex_index_map(get(boost::vertex_index, *halfedge_graph)),
          hedge_index_map(get(boost::halfedge_index, *halfedge_graph)),
          vertex_point_map(get(CGAL::vertex_point, *halfedge_graph)),
          id_to_ros_id_map(std::vector<std::size_t>(num_vertices(*halfedge_graph), (std::numeric_limits<std::size_t>::max)() )),
          is_roi_map(std::vector<bool>(num_vertices(*halfedge_graph), false)),
          is_ctrl_map(std::vector<bool>(num_vertices(*halfedge_graph), false)),
          m_iterations(5), m_tolerance(1e-4),
          need_preprocess_factorization(true),
          need_preprocess_region_of_solution(true),
          last_preprocess_successful(false),
          m_color(0.4f, 0.4f, 0.4f, 1.f),
          weight_calculator(Weight_calculator(*m_halfedge_graph))
    {
        m_halfedge_graph.reset(halfedge_graph);
        m_glVBO[0] = 0;
        m_glVBO[1] = 0;
        init();
    }
    virtual ~MyPolyhedron()
    {
        deleteVBO();
    }
    void init()
    {
        set_halfedgeds_items_id_normal(*m_halfedge_graph);
        //compute halfedge weights
        halfedge_iterator eb, ee;
        hedge_weight.reserve(2*num_edges(*m_halfedge_graph));
        for(CGAL::cpp11::tie(eb, ee) = halfedges(*m_halfedge_graph); eb != ee; ++eb)
        {
            hedge_weight.push_back(this->weight_calculator(*eb));
        }
    }
    virtual glm::vec3
    getCentroid( const glm::mat4 &ctm )
    {
        glm::vec3 c(0,0,0);
        vertex_iterator vb, ve;
        for(boost::tie(vb, ve) = vertices(*m_halfedge_graph); vb != ve; ++vb)
        {
            vertex_descriptor vd = *vb;
            Point p = vd->point();
            glm::vec4 point = ctm * glm::vec4( p.x(), p.y(), p.z(), 1.f );
            c += glm::vec3(point.x, point.y, point.z);
        }
        return c / (float)num_vertices(*m_halfedge_graph);
    }
    void deleteVBO()
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
    bool hasVBO() const
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

    void buildVBO()
    {
        deleteVBO();


        glm::vec3 *data = new glm::vec3[6*num_faces(*m_halfedge_graph)];
        glm::vec3 *edgeData = new glm::vec3[2*num_edges(*m_halfedge_graph)];
        int index = 0, edgeIndex = 0;

        face_iterator fb, fe;
        for(boost::tie(fb, fe) = faces(*m_halfedge_graph); fb != fe; ++fb)
          {
            halfedge_descriptor edg = halfedge(*fb, *m_halfedge_graph);
            halfedge_descriptor edgb = edg;

            Point p0 = vertex_point_map[target(edg, *m_halfedge_graph)];

            Vector n0 = target(edg, *m_halfedge_graph)->normal();
            edg = next(edg, *m_halfedge_graph);


            Point p1 = vertex_point_map[target(edg, *m_halfedge_graph)];
            Vector n1 = target(edg, *m_halfedge_graph)->normal();
            edg = next(edg, *m_halfedge_graph);



            Point p2 = vertex_point_map[target(edg, *m_halfedge_graph)];
            Vector n2 = target(edg, *m_halfedge_graph)->normal();
            edg = next(edg, *m_halfedge_graph);

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
        for(boost::tie(eb, ee) = edges(*m_halfedge_graph); eb != ee; ++eb)
        {
            Point p0 = vertex_point_map[source(*eb, *m_halfedge_graph)];
            Point p1 = vertex_point_map[target(*eb, *m_halfedge_graph)];
            edgeData[edgeIndex++] = glm::vec3(p0.x(), p0.y(), p0.z());
            edgeData[edgeIndex++] = glm::vec3(p1.x(), p1.y(), p1.z());

        }


        // Build OpenGL VBO
        glGenBuffers( 2, &m_glVBO[0] );
        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[0] );
        glBufferData( GL_ARRAY_BUFFER, 6*num_faces(*m_halfedge_graph)*sizeof(glm::vec3), data, GL_STATIC_DRAW );

//        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[1] );
//        glBufferData( GL_ARRAY_BUFFER, 2*num_edges(*m_halfedge_graph)*sizeof(glm::vec3), edgeData, GL_STATIC_DRAW );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );

        delete [] data;
    }


    virtual void render()
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
    void renderVBO()
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[0] );
        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 2*sizeof(glm::vec3), (void*)(0) );
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( GL_FLOAT, 2*sizeof(glm::vec3), (void*)(sizeof(glm::vec3)) );

        glDrawArrays( GL_TRIANGLES, 0, 3 * num_faces(*m_halfedge_graph) );

        glDisableClientState( GL_NORMAL_ARRAY );


//        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(glm::vec4(0, 0, 0, 1)) );
//        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(glm::vec4(0, 0, 0, 1)) );

//        glBindBuffer( GL_ARRAY_BUFFER, m_glVBO[1] );
//        glEnableClientState( GL_VERTEX_ARRAY );
//        glVertexPointer( 3, GL_FLOAT, 0, (void*)(0) );

//        glDrawArrays( GL_LINES, 0, 2 * num_edges(*m_halfedge_graph) );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glDisableClientState( GL_VERTEX_ARRAY );


    }
    virtual void renderForPicker()
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

    std::size_t hd_to_id(halfedge_descriptor hd) const
    {
      return get(hedge_index_map, hd);
    }
    std::size_t vd_to_id(vertex_descriptor vd) const
    {
      return get(vertex_index_map, vd);
    }

    std::size_t& vd_to_ros_id(vertex_descriptor vd)
    {
        return id_to_ros_id_map[vd_to_id(vd)];
    }

    std::size_t vd_to_ros_id(vertex_descriptor vd) const
    {
        return id_to_ros_id_map[vd_to_id(vd)];
    }

    bool is_roi_vertex(vertex_descriptor vd) const
    { return is_roi_map[vd_to_id(vd)]; }
    bool is_control_vertex(vertex_descriptor vd) const
    { return is_ctrl_map[vd_to_id(vd)]; }

    void need_preprocess_both()
    {
      need_preprocess_factorization = true;
      need_preprocess_region_of_solution = true;
    }
    bool insert_roi_vertex(int i)
    {
        vertex_descriptor vd = id_to_vd(i);
        if( is_roi_vertex(vd))
            return false;
        need_preprocess_both();
        is_roi_map[i] = true;
        roi.push_back(vd);
        return true;

    }

    bool erase_roi_vertex(int i)
    {
        vertex_descriptor vd = id_to_vd(i);
      if(!is_roi_vertex(vd)) { return false; }

      erase_control_vertex(i); // also erase from being control

      typename std::vector<vertex_descriptor>::iterator it = std::find(roi.begin(), roi.end(), vd);
      if(it != roi.end())
      {
        is_roi_map[i] = false;
        roi.erase(it);

        need_preprocess_both();
        return true;
      }

      CGAL_assertion(false); // inconsistency between is_roi_map, and roi vector!
      return false;
    }

    bool insert_control_vertex(int i)
    {
      vertex_descriptor vd = id_to_vd(i);
      if(is_control_vertex(vd)) { return false; }
      need_preprocess_factorization=true;

      insert_roi_vertex(i); // also insert it as roi

      is_ctrl_map[i] = true;
      return true;
    }

    bool erase_control_vertex(int i)
    {
      vertex_descriptor vd = id_to_vd(i);
      if(!is_control_vertex(vd)) { return false; }

      need_preprocess_factorization=true;
      is_ctrl_map[i] = false;
      return true;
    }

    void set_target_vertex(int i, Vertex pos)
    {
        vertex_descriptor vd = id_to_vd(i);
        region_of_solution(); // we require ros ids, so if there is any need to preprocess of region of solution -do it.

        if(!is_control_vertex(vd)) { return; }
        Point p(pos.x, pos.y, pos.z);
        solution[vd_to_ros_id(vd)] = p;
    }


    vertex_descriptor id_to_vd(int i)
    {
        vertex_iterator vd = vertices(*m_halfedge_graph).first;
        return *CGAL::cpp11::next(vd, i);
    }


    void updateNormals()
    {
        //set facets normal
        std::for_each( m_halfedge_graph->facets_begin(),   m_halfedge_graph->facets_end(),   Facet_normal());
        // set points normal
        std::for_each( m_halfedge_graph->vertices_begin(), m_halfedge_graph->vertices_end(), Vertex_normal());
    }

    void deform()
    {
      deform(m_iterations, m_tolerance);
      updateNormals();
    }


    void deform(unsigned int iterations, double tolerance)
    {
      preprocess();

      if(!last_preprocess_successful) {
        CGAL_warning(false);
        return;
      }
      // Note: no energy based termination occurs at first iteration
      // because comparing energy of original model (before deformation) and deformed model (deformed_1_iteration)
      // simply does not make sense, comparison is meaningful between deformed_(i)_iteration & deformed_(i+1)_iteration

      double energy_this = 0; // initial value is not important, because we skip first iteration
      double energy_last;

      // iterations
      for ( unsigned int ite = 0; ite < iterations; ++ite)
      {
        // main steps of optimization
        update_solution();

        optimal_rotations();

        // energy based termination
        if(tolerance > 0.0 && (ite + 1) < iterations) // if tolerance <= 0 then don't compute energy
        {                                             // also no need compute energy if this iteration is the last iteration
          energy_last = energy_this;
          energy_this = energy();
          CGAL_warning(energy_this >= 0);

          if(ite != 0) // skip first iteration
          {
            double energy_dif = std::abs((energy_last - energy_this) / energy_this);
            if ( energy_dif < tolerance ) { break; }
          }
        }
      }
      // copy solution to the target surface mesh
      assign_solution();
    }

    void assign_solution()
    {
      for(std::size_t i = 0; i < ros.size(); ++i){
        std::size_t v_id = vd_to_ros_id(ros[i]);
        if(is_roi_vertex(ros[i]))
        {
          put(vertex_point_map, ros[i], solution[v_id]);
        }
      }
    }

    double energy() const
    {
        return energy_arap();
        return 0;
    }

    double energy_arap() const
    {
      Closest_rotation_traits cr_traits;

      double sum_of_energy = 0;
      // only accumulate ros vertices
      for( std::size_t k = 0; k < ros.size(); k++ )
      {
        vertex_descriptor vi = ros[k];
        std::size_t vi_id = vd_to_ros_id(vi);

        in_edge_iterator e, e_end;
        for (CGAL::cpp11::tie(e,e_end) = in_edges(vi, *m_halfedge_graph); e != e_end; e++)
        {
          halfedge_descriptor he = halfedge(*e, *m_halfedge_graph);
          vertex_descriptor vj = source(he, *m_halfedge_graph);
          std::size_t vj_id = vd_to_ros_id(vj);

          const CR_vector& pij = sub_to_CR_vector(original[vi_id], original[vj_id]);
          const CR_vector& qij = sub_to_CR_vector(solution[vi_id], solution[vj_id]);

          double wij = hedge_weight[hd_to_id(he)];

          sum_of_energy += wij * cr_traits.squared_norm_vector_scalar_vector_subs(qij, rot_mtr[vi_id], pij);
          // sum_of_energy += wij * ( qij - rot_mtr[vi_id]*pij )^2
        }
      }
      return sum_of_energy;
    }

    void optimal_rotations()
    {
        optimal_rotations_arap();
    }

    void optimal_rotations_arap()
    {
      Closest_rotation_traits cr_traits;
      CR_matrix cov = cr_traits.zero_matrix();

      // only accumulate ros vertices
      for ( std::size_t k = 0; k < ros.size(); k++ )
      {
        vertex_descriptor vi = ros[k];
        std::size_t vi_id = vd_to_ros_id(vi);
        // compute covariance matrix (user manual eq:cov_matrix)
        cov = cr_traits.zero_matrix();

        in_edge_iterator e, e_end;

        for (CGAL::cpp11::tie(e,e_end) = in_edges(vi, *m_halfedge_graph); e != e_end; e++)
        {
          halfedge_descriptor he=halfedge(*e, *m_halfedge_graph);
          vertex_descriptor vj = source(he, *m_halfedge_graph);
          std::size_t vj_id = vd_to_ros_id(vj);

          const CR_vector& pij = sub_to_CR_vector(original[vi_id], original[vj_id]);
          const CR_vector& qij = sub_to_CR_vector(solution[vi_id], solution[vj_id]);
          double wij = hedge_weight[hd_to_id(he)];

          cr_traits.add_scalar_t_vector_t_vector_transpose(cov, wij, pij, qij); // cov += wij * (pij * qij)

        }

        cr_traits.compute_close_rotation(cov, rot_mtr[vi_id]);
      }
    }
    void update_solution()
    {
        update_solution_arap();
    }


    void update_solution_arap()
    {
      typename Sparse_linear_solver::Vector X(ros.size()), Bx(ros.size());
      typename Sparse_linear_solver::Vector Y(ros.size()), By(ros.size());
      typename Sparse_linear_solver::Vector Z(ros.size()), Bz(ros.size());

      Closest_rotation_traits cr_traits;

      // assemble right columns of linear system
      for ( std::size_t k = 0; k < ros.size(); k++ )
      {
        vertex_descriptor vi = ros[k];
        std::size_t vi_id = vd_to_ros_id(vi);

        if ( is_roi_vertex(vi) && !is_control_vertex(vi) )
        {// free vertices
          // sum of right-hand side of eq:lap_ber in user manual
          CR_vector xyz = cr_traits.vector(0, 0, 0);

          in_edge_iterator e, e_end;
          for (CGAL::cpp11::tie(e,e_end) = in_edges(vi, *m_halfedge_graph); e != e_end; e++)
          {
            halfedge_descriptor he = halfedge(*e, *m_halfedge_graph);
            vertex_descriptor vj = source(he, *m_halfedge_graph);
            std::size_t vj_id = vd_to_ros_id(vj);

            const CR_vector& pij = sub_to_CR_vector(original[vi_id], original[vj_id]);

            double wij = hedge_weight[hd_to_id(he)];
            double wji = hedge_weight[hd_to_id(opposite(he, *m_halfedge_graph))];

            cr_traits.add__scalar_t_matrix_p_scalar_t_matrix__t_vector(xyz, wij, rot_mtr[vi_id], wji, rot_mtr[vj_id], pij);

            // corresponds xyz += (wij*rot_mtr[vi_id] + wji*rot_mtr[vj_id]) * pij
          }
          Bx[vi_id] = cr_traits.vector_coordinate(xyz, 0);
          By[vi_id] = cr_traits.vector_coordinate(xyz, 1);
          Bz[vi_id] = cr_traits.vector_coordinate(xyz, 2);
        }
        else
        {// constrained vertex
          Bx[vi_id] = solution[vi_id][0]; By[vi_id] = solution[vi_id][1]; Bz[vi_id] = solution[vi_id][2];
        }
      }

      // solve "A*X = B".
      bool is_all_solved = m_solver.linear_solver(Bx, X) && m_solver.linear_solver(By, Y) && m_solver.linear_solver(Bz, Z);
      if(!is_all_solved) {
        CGAL_warning(false);
        return;
      }
      // copy to solution
      for (std::size_t i = 0; i < ros.size(); i++)
      {
        std::size_t v_id = vd_to_ros_id(ros[i]);
        Point p(X[v_id], Y[v_id], Z[v_id]);
        solution[v_id] = p;
      }
    }
    CR_vector sub_to_CR_vector(const Point& p1, const Point& p2) const
    {
      return Closest_rotation_traits().vector(p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]);
    }

    bool preprocess()
    {
      region_of_solution();
      assemble_laplacian_and_factorize();
      return last_preprocess_successful; // which is set by assemble_laplacian_and_factorize()
    }

    void assign_ros_id_to_one_ring(vertex_descriptor vd,
                               std::size_t& next_id,
                               std::vector<vertex_descriptor>& push_vector)
    {
      in_edge_iterator e, e_end;
      for (CGAL::cpp11::tie(e,e_end) = in_edges(vd, *m_halfedge_graph); e != e_end; e++)
      {
        vertex_descriptor vt = source(*e, *m_halfedge_graph);
        if(vd_to_ros_id(vt) == (std::numeric_limits<std::size_t>::max)())  // neighboring vertex which is outside of roi and not visited previously (i.e. need an id)
        {
          vd_to_ros_id(vt) = next_id++;
          push_vector.push_back(vt);
        }
      }
    }

    void region_of_solution()
    {
      if(!need_preprocess_region_of_solution) { return; }
      need_preprocess_region_of_solution = false;

      std::vector<std::size_t>  old_id_to_ros_id_map = id_to_ros_id_map;
      std::vector<CR_matrix>    old_rot_mtr    = rot_mtr;
      std::vector<Point>        old_solution   = solution;
      std::vector<Point>        old_original   = original;

      // any vertices which are no longer ROI, should be assigned to their original position, so that:
      // IF a vertex is ROI (actually if its ros + boundary) previously (when previous region_of_solution() is called)
      // we have its original position in old_original
      // ELSE
      // we have its original position in vertex->point()
      // (current ros is actually old ros - we did not change it yet)
      for(typename std::vector<vertex_descriptor>::iterator it = ros.begin(); it != ros.end(); ++it)
      {
        if(!is_roi_vertex(*it)) {
          put(vertex_point_map, *it, old_original[ old_id_to_ros_id_map[vd_to_id(*it)] ]);
        }
      }

      ////////////////////////////////////////////////////////////////
      // assign id to vertices inside: roi, boundary of roi (roi + boundary of roi = ros),
      //                               and boundary of ros
      // keep in mind that id order does not have to be compatible with ros order
      ros.clear(); // clear ros
      ros.insert(ros.end(), roi.begin(), roi.end());

      id_to_ros_id_map.assign(num_vertices(*m_halfedge_graph), (std::numeric_limits<std::size_t>::max)()); // use max as not assigned mark

      for(std::size_t i = 0; i < roi.size(); i++)  // assign id to all roi vertices
      { vd_to_ros_id(roi[i]) = i; }

      // now assign an id to vertices on boundary of roi
      // Basically, here we are only instereted ROI vertices, however, in order to build the laplacian
      // matrix of ROI vertices, their one-ring neighbor are required.
      std::size_t next_ros_index = roi.size();
      for(std::size_t i = 0; i < roi.size(); i++)
      { assign_ros_id_to_one_ring(roi[i], next_ros_index, ros); }

      std::vector<vertex_descriptor> outside_ros;
      // boundary of ros also must have ids because in CR calculation,
      // one-ring neighbor of ROS vertices are reached.
      // In other words, when we calculate the closest rotation matrix of ROS vertices, their one-ring
      // neighbor are used.
      for(std::size_t i = roi.size(); i < ros.size(); i++)
      { assign_ros_id_to_one_ring(ros[i], next_ros_index, outside_ros); }//second ring
      ////////////////////////////////////////////////////////////////

      // initialize the rotation matrices (size: ros)
      rot_mtr.resize(ros.size());
      for(std::size_t i = 0; i < rot_mtr.size(); i++)
      {
        std::size_t v_ros_id = vd_to_ros_id(ros[i]);
        std::size_t v_id = vd_to_id(ros[i]);

        // any vertex which is previously ROS has a rotation matrix
        // use that matrix to prevent jumping effects
        if(old_id_to_ros_id_map[v_id] != (std::numeric_limits<std::size_t>::max)()
            && old_id_to_ros_id_map[v_id] < old_rot_mtr.size()) {
            // && boundary of ros vertices also have ids so check whether it is ros
          rot_mtr[v_ros_id] = old_rot_mtr[ old_id_to_ros_id_map[v_id] ];
        }
        else {
          rot_mtr[v_ros_id] = Closest_rotation_traits().identity_matrix();
        }
      }

      // initialize solution and original (size: ros + boundary_of_ros)

      // for simplifying coding effort, I also put boundary of ros into solution and original
      // because boundary of ros vertices are reached in optimal_rotations() and energy()
      solution.resize(ros.size() + outside_ros.size());
      original.resize(ros.size() + outside_ros.size());

      for(std::size_t i = 0; i < ros.size(); i++)
      {
        std::size_t v_ros_id = vd_to_ros_id(ros[i]);
        std::size_t v_id = vd_to_id(ros[i]);

        if(is_roi_vertex(ros[i]) && old_id_to_ros_id_map[v_id] != (std::numeric_limits<std::size_t>::max)()) {
          // if it is currently roi and previously ros + boundary
          // (actually I just need to assign old's to new's if a vertex is currently and previously ROI
          // but no harm on assigning if its also previously ros + boundary because
          // those(old_original & old_solution) will be equal to original position)
          original[v_ros_id] = old_original[old_id_to_ros_id_map[v_id]];
          solution[v_ros_id] = old_solution[old_id_to_ros_id_map[v_id]];
        }
        else {
          solution[v_ros_id] = get(vertex_point_map, ros[i]);
          original[v_ros_id] = get(vertex_point_map, ros[i]);
        }
      }

      for(std::size_t i = 0; i < outside_ros.size(); ++i)
      {
        std::size_t v_ros_id = vd_to_ros_id(outside_ros[i]);
        original[v_ros_id] = get(vertex_point_map, outside_ros[i]);
        solution[v_ros_id] = get(vertex_point_map, outside_ros[i]);
      }

    }

    /// Assemble Laplacian matrix A of linear system A*X=B
    void assemble_laplacian_and_factorize()
    {
        assemble_laplacian_and_factorize_arap();
    }
    /// Construct matrix that corresponds to left-hand side of eq:lap_ber in user manual
    /// Also constraints are integrated as eq:lap_energy_system in user manual
    void assemble_laplacian_and_factorize_arap()
    {
      if(!need_preprocess_factorization) { return; }
      need_preprocess_factorization = false;

      typename Sparse_linear_solver::Matrix A(ros.size());

      /// assign cotangent Laplacian to ros vertices
      for(std::size_t k = 0; k < ros.size(); k++)
      {
        vertex_descriptor vi = ros[k];
        std::size_t vi_id = vd_to_ros_id(vi);
        if ( is_roi_vertex(vi) && !is_control_vertex(vi) )          // vertices of ( roi - ctrl )
        {
          double diagonal = 0;
          in_edge_iterator e, e_end;
          for (CGAL::cpp11::tie(e,e_end) = in_edges(vi, *m_halfedge_graph); e != e_end; e++)
          {
            halfedge_descriptor he = halfedge(*e, *m_halfedge_graph);
            vertex_descriptor vj = source(he, *m_halfedge_graph);
            double wij = hedge_weight[hd_to_id(he)];  // edge(pi - pj)
            double wji = hedge_weight[hd_to_id(opposite(he, *m_halfedge_graph))]; // edge(pi - pj)
            double total_weight = wij + wji;

            A.set_coef(vi_id, vd_to_ros_id(vj), -total_weight, true); // off-diagonal coefficient
            diagonal += total_weight;
          }
          // diagonal coefficient
          A.set_coef(vi_id, vi_id, diagonal, true);
        }
        else
        {
          A.set_coef(vi_id, vi_id, 1.0, true);
        }
      }

      // now factorize
      double D;
      last_preprocess_successful = m_solver.factor(A, D);
      CGAL_warning(last_preprocess_successful);
    }




private:
    shared_ptr<Halfedge_graph> m_halfedge_graph;
    GLuint m_glVBO[2];
    ::Color m_color;
    SceneNode* m_sceneNode;
    Vertex_index_map vertex_index_map;                  ///< storing indices of all vertices
    Hedge_index_map   hedge_index_map;                  ///< storing indices of all halfedges
    Vertex_point_map vertex_point_map;

    std::vector<Point> original;                        ///< original positions of roi (size: ros + boundary_of_ros)
    std::vector<Point> solution;                        ///< storing position of ros vertices during iterations (size: ros + boundary_of_ros)

    std::vector<vertex_descriptor> roi;                 ///< region of interest
    std::vector<vertex_descriptor> ros;                 ///< region of solution, including roi and hard constraints on boundary of roi

    std::vector<std::size_t> id_to_ros_id_map;                ///< (size: num vertices)
    std::vector<bool>        is_roi_map;                ///< (size: num vertices)
    std::vector<bool>        is_ctrl_map;               ///< (size: num vertices)

    std::vector<double> hedge_weight;                   ///< all halfedge weights
    std::vector<CR_matrix> rot_mtr;                     ///< rotation matrices of ros vertices (size: ros)

    Weight_calculator weight_calculator;

    bool need_preprocess_factorization;                 ///< is there any need to compute L and factorize
    bool need_preprocess_region_of_solution;            ///< is there any need to compute region of solution

    unsigned int m_iterations;                          ///< number of maximal iterations
    double m_tolerance;                                 ///< tolerance of convergence

    bool last_preprocess_successful;

    Sparse_linear_solver m_solver;                      ///< linear sparse solver

    friend class ParticleSystem;
};
