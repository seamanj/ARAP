//#ifndef ENGINE_H
//#define ENGINE_H


//#include "particlesystem.h"
//#include <QObject>
//#include <QTimer>
//#include <QVector>
//#include <memory>

//#include "renderable.h"


//using std::shared_ptr;

//class Engine : public QObject
//{
//    Q_OBJECT
//public:
//    Engine();
//    virtual ~Engine();
    
//    bool start();
//    void pause();
//    void resume();
//    void stop();
//    void reset();
    
    
//    float getSimulationTime() { return m_time; }
    
//    bool isRunning();
    
    
//    void addParticleSystem( const shared_ptr<ParticleSystem> &spParticleSystem );
//    void resetParticleSystems();
//signals:
//    void updateTool();
    
//public slots:

//    void update();
//private:

//    QTimer m_ticker;
//    float m_time;
//    QVector<shared_ptr<ParticleSystem>> m_spParticleSystems;
//    bool m_busy;
//    bool m_running;
//    bool m_paused;
    
//    friend class ViewPanel;
//};

//#endif // ENGINE_H
