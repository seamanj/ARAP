#ifndef UISETTINGS_H
#define UISETTINGS_H

#define DEFINE_SETTING( TYPE, NAME )                            \
    private:                                                    \
        TYPE m_##NAME;                                          \
    public:                                                     \
        static TYPE& NAME() { return instance()->m_##NAME; }    \


#include <QPoint>
#include <QSize>
#include <QVariant>

#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"


struct Grid;

class UiSettings
{

public:

    enum MeshMode
    {
        SOLID,
        WIREFRAME,
        POINT
    };

  

public:

    static UiSettings* instance();
    static void deleteInstance();

    static void loadSettings();
    static void saveSettings();

    static QVariant getSetting( const QString &name, const QVariant &d = QVariant() );
    static void setSetting( const QString &name, const QVariant &value );

    static Grid buildGrid( const glm::mat4 &ctm );

protected:

    UiSettings() {}
    virtual ~UiSettings() {}

private:

    static UiSettings *INSTANCE;

    DEFINE_SETTING( QPoint, windowPosition )
    DEFINE_SETTING( QSize, windowSize )
    
    DEFINE_SETTING( int, showMeshesMode)
    DEFINE_SETTING( int, showParticleSystemsMode)

    
    DEFINE_SETTING( bool, showMesh )
    DEFINE_SETTING( bool, showPariticleSystem )
    DEFINE_SETTING( bool, showParticle )
    
    DEFINE_SETTING( glm::vec4, meshSelectionColor )
    DEFINE_SETTING( glm::vec4, particleSystemSelectionColor)
    DEFINE_SETTING( glm::vec4, particleSelectionColor)
    DEFINE_SETTING( glm::vec4, ROIColor )
    DEFINE_SETTING( glm::vec4, controlColor )
    
    DEFINE_SETTING( float, timeStep )
    
    DEFINE_SETTING( float, stiffness )
    DEFINE_SETTING( float, beta )
    
    
};

#endif // UISETTINGS_H
