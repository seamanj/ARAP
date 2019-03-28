#ifndef GRAPHFUNCTION_H
#define GRAPHFUNCTION_H

#include "types.h"
#include "mypolyhedron.h"

template<class HalfedgeDS_with_id_normal>
void set_halfedgeds_items_id_normal ( HalfedgeDS_with_id_normal& hds )
{
  std::size_t vertex_id   = 0 ;
  std::size_t halfedge_id = 0 ;
  std::size_t face_id     = 0 ;

  for ( typename HalfedgeDS_with_id_normal::Vertex_iterator vit = hds.vertices_begin(), evit = hds.vertices_end()
      ; vit != evit
      ; ++  vit
      )
    vit->id() = vertex_id ++ ;

  for ( typename HalfedgeDS_with_id_normal::Halfedge_iterator hit = hds.halfedges_begin(), ehit = hds.halfedges_end()
      ; hit != ehit
      ; ++  hit
      )
    hit->id() = halfedge_id ++ ;

  for ( typename HalfedgeDS_with_id_normal::Face_iterator fit = hds.facets_begin(), efit = hds.facets_end()
      ; fit != efit
      ; ++  fit
      )
    fit->id() = face_id ++ ;

  //set facets normal
  std::for_each( hds.facets_begin(),   hds.facets_end(),   Facet_normal());

  // set points normal
  std::for_each( hds.vertices_begin(), hds.vertices_end(), Vertex_normal());

}


//template < class Traits,
//           class Items,
//           template < class T, class I, class A>
//           class HDS, class Alloc>
//std::istream& operator>>(std::istream& in,
//                         Polyhedron_3<Traits,Items,HDS,Alloc>& P) {
//    // reads a polyhedron from `in' and appends it to P.
//    CGAL::scan_OFF( in, P);
//    return in;
//}





#endif // GRAPHFUNCTION_H
