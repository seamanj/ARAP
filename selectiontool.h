#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include "tool.h"
#include <QRectF>
#include <glm/vec2.hpp>
#include <QtCore/QObject>
class SceneNode;

class SelectionTool : public Tool
{
    Q_OBJECT
public:

    SelectionTool( ViewPanel *panel,Type t);
    virtual ~SelectionTool();

    virtual void mousePressed();
    virtual void mouseMoved();
    virtual void mouseReleased();

    virtual void update() {}

    virtual void render();

    bool hasSelection( glm::vec3 &center ) const;
    bool hasMovableSelection(glm::vec3 &center) const;
    bool hasRotatableSelection( glm::vec3 &center ) const;
    bool hasScalableSelection( glm::vec3 &center ) const;

    void clearSelection();
    SceneNode* getSelectedSceneNode();
    QList<SceneNode*> getSelectedSceneNodes();
    
// public slots:
//    void updateSize(int width, int height);
    
    
protected:
    glm::vec2 m_pressedPos;
    QRectF m_selectionRect;
    
};

#endif // SELECTIONTOOL_H
