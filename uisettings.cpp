#include <QSettings>

#include "common.h"
#include "uisettings.h"

UiSettings* UiSettings::INSTANCE = NULL;

UiSettings*
UiSettings::instance()
{
    if ( !INSTANCE ) {
        INSTANCE = new UiSettings();
    }
    return INSTANCE;
}

void
UiSettings::deleteInstance()
{
    SAFE_DELETE( INSTANCE );
}

QVariant
UiSettings::getSetting( const QString &name, const QVariant &d )
{
    QSettings s( "NCCA", "seamanj" );
    return s.value( name, d );
}

void
UiSettings::setSetting( const QString &name, const QVariant &value )
{
    QSettings s( "NCCA", "seamanj" );
    s.setValue( name, value );
}

void
UiSettings::loadSettings()
{
    QSettings s( "NCCA", "seamanj" );

    windowPosition() = s.value( "windowPosition", QPoint(0,0) ).toPoint();
    windowSize() = s.value( "windowSize", QSize(1200,768) ).toSize();
    showMeshesMode() = s.value( "showMeshesMode", SOLID).toInt();
    showParticleSystemsMode() = s.value("showParticleSystemsMode", SOLID).toInt();
   
    showMesh() = s.value("showMesh", true).toBool();
    showPariticleSystem() = s.value("showPariticleSystem", true).toBool();
    showParticle() = s.value("showParticle", true).toBool();
    meshSelectionColor() = glm::vec4( 0.302f, 0.773f, 0.839f, 1.f );
    particleSystemSelectionColor() = glm::vec4(88.f / 255, 45.f / 255, 216.f /255, 1.0f);
    particleSelectionColor() = glm::vec4(0.929411765f, 0.529411765f, 0.098039216f, 1.0f);
    ROIColor() = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f );
    controlColor() = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    timeStep() = s.value( "timeStep", 1e-2 ).toFloat();
    stiffness() = s.value( "stiffness", 1 ).toFloat();
    beta() = s.value( "beta", 0.2 ).toFloat();
}

void
UiSettings::saveSettings()
{
    QSettings s( "NCCA", "seamanj" );

    s.setValue( "windowPosition", windowPosition() );
    s.setValue( "windowSize", windowSize());
    s.setValue( "showMeshesMode", showMeshesMode());//save value in uisetting to qsetting
    s.setValue( "showParticleSystemsMode", showParticleSystemsMode());
   
    s.setValue( "showMesh", showMesh());
    s.setValue( "showPariticleSystem", showPariticleSystem());
    s.setValue( "showParticle", showParticle());
    s.setValue( "timeStep", timeStep() );
    s.setValue( "stiffness", stiffness() );
    s.setValue( "beta", beta() );
}


