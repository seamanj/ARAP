#ifndef VIEWPANEL_H
#define VIEWPANEL_H
#include "mypolyhedron.h"
#include "engine.h"
#include <QGLWidget>
#include <QElapsedTimer>
#include <QTimer>
#include "mesh.h"
#include "scene.h"
#include "viewport.h"
#include "tool.h"
#include "particlesystem.h"
#include <memory>
#include "mainwindow.h"



using std::shared_ptr;
class InfoPanel;


class ViewPanel : public QGLWidget
{
    Q_OBJECT
public:
    ViewPanel(QWidget *parent = 0);
    virtual ~ViewPanel();
    void paintGrid();
    bool hasGridVBO() const;
    void buildGridVBO();
    void deleteGridVBO();
    
    bool startSimulation();
    void stopSimulation();

signals:

    void changeSelection(int mode);
    void changeSize(int w, int h);
public slots:
    void polyhedronDeformed();
    void toolUpdated();
    void loadMesh( const QString &filename );
    void saveMesh( const QString &filename );
    virtual void initializeGL();
    virtual void paintGL();
    void resetViewport();

    virtual void resizeEvent( QResizeEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
    virtual void keyPressEvent( QKeyEvent *event );
    void setTool( int tool );
    void checkSelected();
    void clearSelection();

    void particlize();
    void roiSelectedParticles();
    void deroiSelectedParticles();
    void controlSelectedParticles();
    void decontrolSelectedParticles();
    //void deformationModeChanged(int mode);
    void selectPriorGroup();
    void selectNextGroup();


    void resetSimulation();
    void pauseSimulation( bool pause = true );
    void resumeSimulation();
    


protected:
    QTimer m_ticker;
    Scene *m_scene;
    Viewport *m_viewport;
    Tool *m_tool;
    GLuint m_gridVBO;
    int m_majorSize;
    int m_minorSize;
    InfoPanel *m_infoPanel;
    bool m_draw;
    float m_fps;
    QElapsedTimer m_timer;
//    Engine *m_engine;
private:
    //shared_ptr<Surface_mesh_deformation> m_spDeformation;
    //shared_ptr<Polyhedron> m_spPolyhedron;
    QList<shared_ptr<Mesh>> m_spMeshes;
    shared_ptr<MyPolyhedron<Polyhedron>> m_spMyPolyhedron;
    shared_ptr<ParticleSystem> m_spParticleSystem;
    SceneNode *m_selected;
    friend class Tool;
    friend class MoveTool;
    friend class SelectionTool;
    friend class RotateTool;
    friend class ScaleTool;
    friend class MainWindow;  
};

#endif // VIEWPANEL_H
