#ifndef SCENENODEITERATOR_H
#define SCENENODEITERATOR_H

#include <QList>


#include "scenenode.h"


class SceneNodeIterator
{

public:

    SceneNodeIterator() : m_index(0) {}
    SceneNodeIterator( const QList<SceneNode*> &nodes ) : m_nodes(nodes), m_index(0) {}
    SceneNodeIterator( const SceneNodeIterator &other ) : m_nodes(other.m_nodes), m_index(other.m_index) {}

    SceneNodeIterator& operator ++ () { ++m_index; return *this; }
    SceneNodeIterator  operator ++ ( int ) { SceneNodeIterator result(*this); ++(*this); return result; }

    SceneNode* operator * () { return m_nodes[m_index]; }

    bool isValid() const { return m_index < m_nodes.size(); }
    void reset() { m_index = 0; }

    int size() const { return m_nodes.size(); }

private:

    QList<SceneNode*> m_nodes;
    int m_index;

};




#endif // SCENENODEITERATOR_H
