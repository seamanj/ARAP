#ifndef OBJPARSER_H
#define OBJPARSER_H
#include <QFile>
#include <QString>
#include <QQueue>
#include <QVector>
#include <QList>
#include "mesh.h"
#include "types.h"
#include <memory>
using std::shared_ptr;
class OBJParser
{
public:

    static void load( const QString &filename, QList<shared_ptr<Mesh>> &spMeshes );
    static bool save( const QString &filename, QList<shared_ptr<Mesh>> &spMeshes );

    OBJParser(const QString &filename = QString()):m_file(filename) {}
    virtual ~OBJParser(){clear();}
    bool load();
    bool save();
    void clear();
    inline bool hasMeshes() const { return !m_spMeshes.empty(); }
    inline shared_ptr<Mesh> popMesh() { return m_spMeshes.dequeue(); }
    inline void setMeshes( const QList<shared_ptr<Mesh>> &spMeshes ) { m_spMeshes.clear(); m_spMeshes += spMeshes; }
    bool parse( const QStringList &lines, int &lineIndex );
    bool parseName( const QString &line );
    bool parseVertex( const QString &line );
    bool parseFace( const QString &line );
private:
    enum Mode { VERTEX, FACE, GROUP };
    Mode m_mode;
    QFile m_file;
    QVector<Vertex> m_vertexPool;
    QVector<Tri> m_triPool;
    QVector<Normal> m_normalPool;
    QString m_currentName;// the name of object we are dealing with
    QQueue<shared_ptr<Mesh>> m_spMeshes;

    inline bool meshPending() const { return !m_vertexPool.empty() && !m_triPool.empty(); }
    void addMesh();
    void setMode( Mode mode );
    QString write( shared_ptr<Mesh> spMesh ) const;
};

#endif // OBJPARSER_H
