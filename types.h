#ifndef TYPES_H
#define TYPES_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>


#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
// HalfedgeGraph adapters for Polyhedron_3
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/boost/graph/properties_Polyhedron_3.h>



typedef glm::vec3 Vertex;
typedef glm::vec3 Normal;
typedef glm::vec4 Color;


struct Tri {
    union {
        struct { int a, b, c; };
        int corners[3];
    };
    Tri() : a(-1), b(-1), c(-1) {}
    Tri( int i0, int i1, int i2 ) : a(i0), b(i1), c(i2) {}
    Tri( const Tri &other ) : a(other.a), b(other.b), c(other.c) {}
    inline void reverse() { int tmp = a; a = c; c = tmp; }
    inline void offset( int offset ) { a += offset; b += offset; c += offset; }
    inline int& operator [] ( int i ) { return corners[i]; }
    inline int operator [] ( int i ) const { return corners[i]; }
};

#ifdef HIGH_PRECISION
typedef double ScalarType;
#else
typedef float ScalarType;
#endif

typedef Eigen::Matrix<ScalarType, 3, 3, 0, 3 ,3> EigenMatrix3;
typedef Eigen::Matrix<ScalarType, 2, 2, 0, 2 ,2> EigenMatrix2;
typedef Eigen::Matrix<ScalarType, 9, 9, 0, 9 ,9> EigenMatrix9x9;
typedef Eigen::Matrix<ScalarType, 3, 9, 0, 3 ,9> EigenMatrix3x9;

typedef Eigen::Matrix<ScalarType, 3, 1, 0, 3 ,1> EigenVector3;
typedef Eigen::Matrix<ScalarType, 2, 1, 0, 2 ,1> EigenVector2;
typedef Eigen::Matrix<ScalarType, 9, 1, 0, 9 ,1> EigenVector9;

typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> VectorX;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> Matrix;
typedef Eigen::SparseMatrix<ScalarType> SparseMatrix;
typedef Eigen::Triplet<ScalarType,unsigned int> SparseMatrixTriplet;
#define block_vector(a) block<3,1>(3*(a), 0)
#define block_vector9x1(a) block<9,1>(9*(a),0)


typedef CGAL::Simple_cartesian<double>                                   Kernel;
typedef Kernel::Vector_3                                             Vector;
typedef Kernel::Point_3                                                Point;
struct Facet_normal {
    template <class Facet>
    void operator()( Facet& f) {
        typename Facet::Halfedge_handle h = f.halfedge();
        typename Facet::Normal_3 normal = CGAL::cross_product(
          h->next()->vertex()->point() - h->vertex()->point(),
          h->next()->next()->vertex()->point() - h->next()->vertex()->point());
        f.normal() = normal / std::sqrt( normal * normal);
    }
};

struct Vertex_normal {
    template <class Vertex>
    void operator()( Vertex& v) {
        typename Vertex::Normal_3 normal = CGAL::NULL_VECTOR;
        typedef typename Vertex::Halfedge_around_vertex_const_circulator Circ;
        Circ c = v.vertex_begin();
        Circ d = c;
        CGAL_For_all( c, d) {
            if ( ! c->is_border())
                normal = normal + c->facet()->normal();
        }
        v.normal() = normal / std::sqrt( normal * normal);
    }
};

template <class Refs, class P, class Norm, class ID>
class My_vertex : public CGAL::HalfedgeDS_vertex_max_base_with_id<Refs, P, ID> {
    Norm  norm;
public:
    My_vertex() {} // repeat mandatory constructors
    My_vertex( const P& pt) : CGAL::HalfedgeDS_vertex_max_base_with_id<Refs, P, ID>(pt) {}

    typedef Norm Normal_3;
    Normal_3&       normal()       { return norm; }
    const Normal_3& normal() const { return norm; }
};


template <class Refs, class P, class Norm, class ID>
class My_facet : public CGAL::HalfedgeDS_face_max_base_with_id<Refs, P, ID> {
    Norm  norm;
public:
    // no constructors to repeat, since only default constructor mandatory
    typedef Norm Normal_3;
    Normal_3&       normal()       { return norm; }
    const Normal_3& normal() const { return norm; }
};




struct My_items : public CGAL::Polyhedron_items_with_id_3 {
    template <class Refs, class Traits>
    struct Vertex_wrapper {
        typedef typename Traits::Point_3  Point;
        typedef typename Traits::Vector_3 Normal;
        typedef My_vertex<Refs, Point, Normal, std::size_t> Vertex;
    };
    template <class Refs, class Traits>
       struct Face_wrapper {
           typedef typename Traits::Vector_3 Normal;
           typedef My_facet<Refs, CGAL::Tag_true, Normal, std::size_t> Face;
       };

};



typedef CGAL::Polyhedron_3<Kernel, My_items> Polyhedron;

typedef boost::graph_traits<Polyhedron>::vertex_descriptor    vertex_descriptor;
typedef boost::graph_traits<Polyhedron>::vertex_iterator        vertex_iterator;









#endif // TYPES_H
