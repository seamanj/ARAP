#include <GL/glew.h>
#include <GL/gl.h>
#include <algorithm>
#include "viewpanel.h"
#include "mesh.h"
#include "objparser.h"
#include "common.h"
#include "scenenode.h"
#include "viewport.h"
#include "userinput.h"
#include "selectiontool.h"
#include "rotatetool.h"
#include "movetool.h"
#include "scaletool.h"
#include "mymath.h"
#include "infopanel.h"
#include <fstream>
#include "graphfunction.h"

#include <memory>
using std::shared_ptr;
using std::dynamic_pointer_cast;

#define FPS 60


#define MAJOR_GRID_N 5
#define MAJOR_GRID_TICK 1.25
#define MINOR_GRID_TICK 0.25

ViewPanel::ViewPanel(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent), 
      m_infoPanel(NULL),
      m_tool(NULL)
{
    m_viewport = new Viewport;
    resetViewport();

    m_infoPanel = new InfoPanel(this);
    m_infoPanel->setInfo( "Major Grid Unit", QString::number(MAJOR_GRID_TICK) + " m");
    m_infoPanel->setInfo( "Minor Grid Unit", QString::number(100*MINOR_GRID_TICK) + " cm");
    m_infoPanel->setInfo( "FPS", "XXXXXX" );
    //m_infoPanel->setInfo( "Sim Time", "XXXXXXX" );
    
    

    m_draw = true;
    m_fps = FPS;
    
    m_scene = new Scene;
    assert( connect(m_scene, SIGNAL(updateTool()), this, SLOT(toolUpdated())));

//     assert( connect(this, SIGNAL(changeSize(int,int)), m_tool, SLOT(updateSize(int,int))));


//    assert(this->disconnect(m_tool));
//    m_engine = new Engine;
    
    makeCurrent();
    glewInit();
}


ViewPanel::~ViewPanel()
{
    makeCurrent();
    deleteGridVBO();
//    SAFE_DELETE( m_engine );
    SAFE_DELETE( m_viewport );
    SAFE_DELETE( m_scene );
    SAFE_DELETE( m_tool );
    SAFE_DELETE( m_infoPanel );
}

void
ViewPanel::resetViewport()
{
    m_viewport->orient( glm::vec3( 10, 10, 10 ),
                        glm::vec3( 0, 0, 0 ),
                        glm::vec3( 0, 1, 0 ) );
    m_viewport->setDimensions( width(), height() );
}

void ViewPanel::loadMesh( const QString &filename)
{
    std::ifstream input(filename.toUtf8().constData());
    Polyhedron *pPolyhedron = new Polyhedron();
    if ( !input || !(input >> *pPolyhedron) || pPolyhedron->empty() )
    {
        LOG( "Cannot open %s", STR(filename) );
    }
    else
    {
        m_spMyPolyhedron.reset(new MyPolyhedron<Polyhedron>(pPolyhedron));
        m_spMyPolyhedron->setSelected(true);
        SceneNode *node = new SceneNode( SceneNode::Mesh );
        node->setRenderable( m_spMyPolyhedron );
        m_spMyPolyhedron->setSceneNode(node);
        m_scene->root()->addChild( node );
    }
//    OBJParser::load( filename, m_spMeshes );

    
//    for ( int i = 0; i < m_spMeshes.size(); ++i ) {
//        shared_ptr<Mesh> spMesh(m_spMeshes[i]);
//        spMesh->setSelected(true);
//        SceneNode *node = new SceneNode( SceneNode::Mesh );
//        node->setRenderable( spMesh );
//        spMesh->setSceneNode(node);
//        m_scene->root()->addChild( node );
//    }

}


void ViewPanel::particlize()
{
    m_spParticleSystem.reset(new ParticleSystem());
    
    for ( SceneNodeIterator it = m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() && (*it)->getType() == SceneNode::Mesh && (*it)->getRenderable()->isSelected()) {
            m_spParticleSystem->initFromPolyhedron( dynamic_pointer_cast<MyPolyhedron<Polyhedron>>((*it)->getRenderable()) );
//            m_engine->addParticleSystem(spParticleSystem);
            SceneNode *node = new SceneNode( SceneNode::ParticleSystem );
            node->setRenderable(m_spParticleSystem);
            m_spParticleSystem->setSceneNode(node);
            m_scene->root()->addChild( node );
            QMap<int, shared_ptr<Particle>>& particles = m_spParticleSystem->getParticles();
            QMap<int, shared_ptr<Particle>>::iterator i;
            for( i = particles.begin(); i != particles.end(); ++i)
            {
                SceneNode *subNode = new SceneNode( SceneNode::Particle );
                subNode->setRenderable(i.value());
                i.value()->setSceneNode(subNode);
                node->addChild(subNode);
            }
            
        }
    }
    m_scene->deleteSelectedMeshes();
    if ( m_tool ) m_tool->update();
}
//void ViewPanel::deformationModeChanged(int mode)
//{
//    for ( SceneNodeIterator it = m_scene->begin(); it.isValid(); ++it ) {
//        if ( (*it)->hasRenderable() && (*it)->getType() == SceneNode::ParticleSystem && (*it)->getRenderable()->isSelected())
//        {
//            dynamic_pointer_cast<ParticleSystem>((*it)->getRenderable())->setDeformationMode(mode);
//        }
//    }
//}

void ViewPanel::roiSelectedParticles()
{
    m_scene->roiSelectedParticles();
}
void ViewPanel::deroiSelectedParticles()
{
    m_scene->deroiSelectedParticles();
}
void ViewPanel::controlSelectedParticles()
{
    m_scene->controlSelectedParticles();
}
void ViewPanel::decontrolSelectedParticles()
{
    m_scene->decontrolSelectedParticles();
}
void ViewPanel::selectPriorGroup()
{
    m_scene->selectPriorGroup();
    if ( m_tool ) m_tool->update();
}
void ViewPanel::selectNextGroup()
{
    m_scene->selectNextGroup();
    if ( m_tool ) m_tool->update();
}


void ViewPanel::saveMesh(const QString &filename)
{
//    if ( !m_spMeshes.empty() )  {
//        if ( !filename.isNull() ) {
//            if ( OBJParser::save( filename, m_spMeshes ) ) {
////                for ( int i = 0; i < m_meshes.size(); ++i )
////                    delete m_meshes[i];
////                m_meshes.clear();
//                m_spMeshes.clear();
//                LOG( "Mesh saved to %s", STR(filename) );
//            }
//        }
//    }
}


void
ViewPanel::initializeGL()
{
    // OpenGL states

    QGLWidget::initializeGL();

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

    
    // Render ticker
    assert( connect(&m_ticker, SIGNAL(timeout()), this, SLOT(update())) );
    m_ticker.start( 1000/FPS );

    m_timer.start();
}

void
ViewPanel::paintGrid()
{
    if ( !hasGridVBO() ) buildGridVBO();

    glPushAttrib( GL_COLOR_BUFFER_BIT );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glBindBuffer( GL_ARRAY_BUFFER, m_gridVBO );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof(glm::vec3), (void*)(0) );
    glColor4f( 0.5f, 0.5f, 0.5f, 0.8f );
    glLineWidth( 2.5f );
    glDrawArrays( GL_LINES, 0, 4 );
    glColor4f( 0.5f, 0.5f, 0.5f, 0.65f );
    glLineWidth( 1.5f );
    glDrawArrays( GL_LINES, 4, m_majorSize-4 );
    glColor4f( 0.5f, 0.5f, 0.5f, 0.5f );
    glLineWidth( 0.5f );
    glDrawArrays( GL_LINES, m_majorSize, m_minorSize );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableClientState( GL_VERTEX_ARRAY );
    glEnd();
    glPopAttrib();
}

bool
ViewPanel::hasGridVBO() const
{
    return m_gridVBO > 0 && glIsBuffer( m_gridVBO );
}

void
ViewPanel::buildGridVBO()
{
    deleteGridVBO();

    QVector<glm::vec3> data;

    static const int minorN = MAJOR_GRID_N * MAJOR_GRID_TICK / MINOR_GRID_TICK;
    static const float max = MAJOR_GRID_N * MAJOR_GRID_TICK;
    for ( int i = 0; i <= MAJOR_GRID_N; ++i ) {
        float x = MAJOR_GRID_TICK * i;
        data += glm::vec3( x, 0.f, -max );
        data += glm::vec3( x, 0.f, max );
        data += glm::vec3( -max, 0.f, x );
        data += glm::vec3( max, 0.f, x );
        if ( i ) {
            data += glm::vec3( -x, 0.f, -max );
            data += glm::vec3( -x, 0.f, max );
            data += glm::vec3( -max, 0.f, -x );
            data += glm::vec3( max, 0.f, -x );
        }
    }
    m_majorSize = data.size();

    for ( int i = -minorN; i <= minorN; ++i ) {
        float x = MINOR_GRID_TICK * i;
        data += glm::vec3( x, 0.f, -max );
        data += glm::vec3( x, 0.f, max );
        data += glm::vec3( -max, 0.f, x );
        data += glm::vec3( max, 0.f, x );
    }
    m_minorSize = data.size() - m_majorSize;

    glGenBuffers( 1, &m_gridVBO );
    glBindBuffer( GL_ARRAY_BUFFER, m_gridVBO );
    glBufferData( GL_ARRAY_BUFFER, data.size()*sizeof(glm::vec3), data.data(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}
void
ViewPanel::deleteGridVBO()
{
    if ( hasGridVBO() ) {
        glDeleteBuffers( 1, &m_gridVBO );
    }
    m_gridVBO = 0;
}
void
ViewPanel::paintGL()
{
    glClearColor( 0.20f, 0.225f, 0.25f, 1.f );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushAttrib( GL_TRANSFORM_BIT );
    glEnable( GL_NORMALIZE );

    m_viewport->push(); {
        m_scene->render();
        paintGrid();
        if ( m_tool ) m_tool->render();
        m_viewport->drawAxis();

    } m_viewport->pop();

    
    if ( m_draw ) {
        static const float filter = 0.8f;
        m_fps = (1-filter)*(1000.f/std::max(m_timer.restart(),(qint64)1)) + filter*m_fps;
        m_infoPanel->setInfo( "FPS", QString::number(m_fps, 'f', 2), false );
        //m_infoPanel->setInfo( "Sim Time", QString::number(m_engine->getSimulationTime(), 'f', 3)+" s", false );
    }
    m_infoPanel->render();
    
    
    glPopAttrib();
}
void ViewPanel::toolUpdated()
{
    if ( m_tool ) m_tool->update();
}
void ViewPanel::setTool( int tool )
{

    if( m_tool && strcmp(typeid(*m_tool).name(), typeid( SelectionTool ).name()) == 0 )
    {
        assert(this->disconnect(m_tool));
    }
    if( m_tool )
    {
        assert( m_tool->disconnect(this));
    }
//    if( m_tool )
//        m_engine->disconnect(m_tool);
    SAFE_DELETE( m_tool );
    Tool::Type t = (Tool::Type)tool;
    switch ( t ) {
    case Tool::SELECTION:
        m_tool = new SelectionTool(this,t);
        assert( connect(this, SIGNAL(changeSize(int,int)), m_tool, SLOT(updateSize(int,int))));
        emit changeSize(width(), height()); 
        break;
    case Tool::MOVE:
        m_tool = new MoveTool(this,t);
        break;
    case Tool::ROTATE:
        m_tool = new RotateTool(this,t);
        break;
    case Tool::SCALE:
        m_tool = new ScaleTool(this,t);
        break;
//    case Tool::VELOCITY:
//        m_tool = new VelocityTool(this,t);
//        break;
    }
//    if( m_tool )
//    {
//        assert( connect(m_engine, SIGNAL(updateTool()), m_tool, SLOT(update())));
//    }
    if( m_tool)
    {
          assert( connect(m_tool, SIGNAL(deformPolyhedron()), this, SLOT(polyhedronDeformed())));
    }
    if ( m_tool ) m_tool->update();
    update();
}
void ViewPanel::polyhedronDeformed()
{
    if( !m_spParticleSystem )
        return;
    else
        m_spParticleSystem->deform();
}
bool ViewPanel::startSimulation()
{
    makeCurrent();
//    if ( !m_engine->isRunning() )
//    {
//        return m_engine->start();
//    }

    return false;
}

void ViewPanel::stopSimulation()
{
//    m_engine->stop();
}


void ViewPanel::pauseSimulation( bool pause )
{
//    if ( pause ) m_engine->pause();
//    else m_engine->resume();
}

void ViewPanel::resumeSimulation()
{
//    m_engine->resume();
}

void ViewPanel::resetSimulation()
{
//    m_engine->reset();
//    if ( m_tool ) m_tool->update();
}


void ViewPanel::checkSelected()  {
   int counter = 0;
   for ( SceneNodeIterator it = m_scene->begin(); it.isValid(); ++it ) {
       if ( (*it)->hasRenderable() &&(*it)->getRenderable()->isSelected() && (*it)->isParticleSystem()) {
           counter++;
           m_selected = (*it);
       }
   }
   if( counter == 1)
       emit changeSelection(dynamic_pointer_cast<ParticleSystem>(m_selected->getRenderable())->getDeformationMode());
   else
   {
       emit changeSelection(0);
       m_selected = NULL;
   }
//   if(counter == 0)  {
//       emit changeSelection(0);
//       m_selected = NULL;
//   }
//   else if(counter == 1 && m_selected->isParticleSystem())  {

//       emit changeSelection(dynamic_pointer_cast<ParticleSystem>(m_selected->getRenderable())->getDeformationMode());
//   }
//   else  {
//       emit changeSelection(0);
//       m_selected = NULL;
//   }
}


void
ViewPanel::clearSelection()
{
    for ( SceneNodeIterator it = m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() ) {
            (*it)->getRenderable()->setSelected( false );
        }
    }
    checkSelected();
}

void
ViewPanel::resizeEvent( QResizeEvent *event )
{
    QGLWidget::resizeEvent( event );
    m_viewport->setDimensions( width(), height() );
    emit changeSize(width(), height());
}
void
ViewPanel::mousePressEvent( QMouseEvent *event )
{
    UserInput::update(event);
    if ( UserInput::ctrlKey() ) {
        if ( UserInput::leftMouse() ) m_viewport->setState( Viewport::TUMBLING );
        else if ( UserInput::rightMouse() ) m_viewport->setState( Viewport::ZOOMING );
        else if ( UserInput::middleMouse() ) m_viewport->setState( Viewport::PANNING );
    } else {
        if ( UserInput::leftMouse() ) if ( m_tool ) m_tool->mousePressed();
    }
    update();
}

void
ViewPanel::mouseMoveEvent( QMouseEvent *event )
{
    UserInput::update(event);
    m_viewport->mouseMoved();
    if ( m_tool ) m_tool->mouseMoved();
    update();
}

void
ViewPanel::mouseReleaseEvent( QMouseEvent *event )
{
    UserInput::update(event);
    m_viewport->setState( Viewport::IDLE );
    if ( m_tool ) m_tool->mouseReleased();
    update();
}

void
ViewPanel::keyPressEvent( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Backspace ) {
        m_scene->deleteSelectedNodes();
        event->accept();
    } else {
        event->setAccepted( false );
    }
    if ( m_tool ) m_tool->update();
    update();
}
