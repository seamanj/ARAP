
#include "types.h"
#include "objparser.h"
#include "common.h"
#include <QRegExp>
#include <QString>
#include <QStringList>

void
OBJParser::load( const QString &filename, QList<shared_ptr<Mesh>> &spMeshes )
{
    spMeshes.clear();
    OBJParser parser( filename );
    if ( parser.load() ) {
        while ( parser.hasMeshes() ) {
            spMeshes += parser.popMesh();
        }
    }
}

bool
OBJParser::save( const QString &filename, QList<shared_ptr<Mesh>> &spMeshes )
{
    OBJParser parser( filename );
    parser.setMeshes( spMeshes );
    return parser.save();
}
void
OBJParser::clear()
{
//    while ( !m_spMeshes.empty() ) {
//        Mesh* mesh = m_spMeshes.dequeue();
//        SAFE_DELETE( mesh );
//    }
    m_spMeshes.clear();
    m_vertexPool.clear();
    m_normalPool.clear();
}
bool
OBJParser::load()
{

    clear();

    if ( m_file.fileName().isEmpty() ) {
        LOG( "OBJParser: No file name!" );
        return false;
    }
    std::cout << "filename:" << m_file.fileName().toStdString() << std::endl;
    if ( !m_file.exists() || !m_file.open(QFile::ReadOnly) ) {
        LOG( "OBJParser: Unable to open file %s.", STR(m_file.fileName()) );
        return false;
    }

    QString text = QString(m_file.readAll());
    m_file.close();

    QStringList lines = text.split( QRegExp("[\r\n]"), QString::SkipEmptyParts );
    int lineIndex = 0;
    while ( lineIndex < lines.size() ) {
        if ( !parse(lines, lineIndex) ) {
            return false;
        }
    }

    if ( meshPending() ) addMesh();

    for ( QQueue<shared_ptr<Mesh>>::iterator it = m_spMeshes.begin(); it != m_spMeshes.end(); ++it ) {
        if ( (*it)->getNumNormals() != (*it)->getNumVertices() ) {
            (*it)->computeNormals();
        }
    }

    return true;

}



bool
OBJParser::save()
{
    if ( m_file.fileName().isEmpty() ) {
        LOG( "OBJParser: No file name!" );
        return false;
    }

    if ( !m_file.open(QFile::WriteOnly) ) {
        LOG( "OBJParser: Unable to open file %s.", STR(m_file.fileName()) );
        return false;
    }

    QString string = "";
    while ( hasMeshes() ) {
        string += write( popMesh() );
    }

    m_file.write( string.toLatin1() );
    m_file.close();

    return true;
}

QString
OBJParser::write( shared_ptr<Mesh> spMesh ) const
{
    char s[1024];
    QString string = "";

    for ( int i = 0; i < spMesh->getNumVertices(); ++i ) {
        const Vertex &v = spMesh->getVertex( i );
        sprintf( s, "v %f %f %f\n", v.x, v.y, v.z );
        string += s;
    }

    string += "g " + spMesh->getName() + "\n";

    for ( int i = 0; i < spMesh->getNumTris(); ++i ) {
        Tri t = spMesh->getTri(i);
        t.offset( 1 ); // OBJ indices start from 1
        sprintf( s, "f %d %d %d\n", t[0], t[1], t[2] );
        string += s;
    }

    string += "\n";

    return string;
}

void
OBJParser::addMesh()
{
    if ( meshPending() ) {
        LOG( "OBJParser: adding mesh %s...", STR(m_currentName) );
        shared_ptr<Mesh> spMesh(new Mesh);
        spMesh->setName( m_currentName );
        spMesh->setFilename( m_file.fileName() );
        spMesh->setVertices( m_vertexPool );
        spMesh->setTris( m_triPool );
        if ( m_normalPool.size() == m_vertexPool.size() )
            spMesh->setNormals( m_normalPool );
        m_spMeshes.enqueue( spMesh );
        m_currentName = "default";
        m_vertexPool.clear();
        m_triPool.clear();
        m_normalPool.clear();
    }
}


bool
OBJParser::parseName( const QString &line )
{
    setMode( GROUP );
    const static QRegExp regExp( "[\\s+\n\r]" );
    QStringList lineStrs = line.split( regExp, QString::SkipEmptyParts );
    m_currentName = ( lineStrs.size() > 1 ) ? lineStrs[1] : "default";
    return true;
}

bool
OBJParser::parseVertex( const QString &line )
{
    setMode( VERTEX );
    const static QRegExp regExp( "[\\s+v]" );//   \\s -  A whitespace [ \t\n\r\f]
    QStringList vertexStrs = line.split( regExp, QString::SkipEmptyParts );
    bool ok[3];
    m_vertexPool += Vertex(vertexStrs[0].toFloat(&ok[0]), vertexStrs[1].toFloat(&ok[1]), vertexStrs[2].toFloat(&ok[2]));
    return ok[0] && ok[1] && ok[2];
}


// Parse face and break into triangles if necessary
bool
OBJParser::parseFace( const QString &line )
{
    setMode( FACE );
    const static QRegExp regExp( "[-\\s+f]" );
    QStringList faceStrs = line.split( regExp, QString::SkipEmptyParts );
    int nCorners = faceStrs.size();
    int *indices = new int[nCorners];
    for ( int i = 0; i < nCorners; ++i ) {
        const static QRegExp regExp2( "[/]" );
        QStringList cornerStrs = faceStrs[i].split( regExp2, QString::KeepEmptyParts );
        bool ok;
        indices[i] = cornerStrs[0].toInt(&ok)-1; // Note: OBJ indices start at 1
        if ( !ok ) return false;
    }
    int nTris = nCorners - 2;
    for ( int i = 0; i < nTris; ++i )
        m_triPool += Tri(indices[0], indices[i+1], indices[i+2]);
    delete [] indices;
    return true;
}



bool
OBJParser::parse( const QStringList &lines,
                  int &lineIndex )
{
    const QString &line = lines[lineIndex++];
    switch ( line[0].toLatin1() ) {
    case '#':
        break;
    case 'g': case 'o':
        if ( !parseName(line) ) {
            LOG( "Error parsing name: %s", STR(line) );
            return false;
        }
        break;
    case 'v':
        switch ( line[1].toLatin1() ) {
        case ' ': case'\t':
            if ( !parseVertex(line) ) {
                LOG( "Error parsing vertex: %s", STR(line) );
                return false;
            }
            break;
        default:
            break;
        }
        break;
    case 'f':
        if ( !parseFace(line) ) {
            LOG( "Error parsing face: %s", STR(line) );
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}


void
OBJParser::setMode( Mode mode )
{
    if ( mode != m_mode ) {
        if ( mode == VERTEX ) {
            addMesh();
        }
        m_mode = mode;
    }
}
