
#ifndef TOOL_H
#define TOOL_H

#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif
#include "glm/mat4x4.hpp"
#include <QtCore/QObject>
#include <QSizeF>
#include "common.h"

class ViewPanel;

class Tool : public QObject
{
    Q_OBJECT
public:

    enum Type
    {
        SELECTION,
        MOVE,
        ROTATE,
        SCALE,
        VELOCITY
    };

    Tool( ViewPanel *panel,Type t ) : m_panel(panel), m_mouseDown(false),m_type(t) {}
    virtual ~Tool() {}

    virtual void mousePressed() { m_mouseDown = true; }
    virtual void mouseMoved() {}
    virtual void mouseReleased() { m_mouseDown = false; }

    

    virtual void render() {}
    
    
    
    static glm::vec3 getAxialColor( unsigned int axis );
public slots:
    void updateSize(int width, int height){m_viewPanelSize = QSizeF(width, height);}
    virtual void update() 
    {
        LOG( "Warning - update : reach <Tool> base class virtual function." );
    }
signals:
    void deformPolyhedron();
protected:
    
    ViewPanel *m_panel;
    bool m_mouseDown;

    static glm::mat4 getAxialBasis( unsigned int axis );

    float getHandleSize( const glm::vec3 &center ) const;

    Type m_type;
    static QSizeF m_viewPanelSize;
};

#endif // TOOL_H
