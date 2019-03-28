#ifndef SCENE_H
#define SCENE_H

#include "scenenode.h"
#include "scenenodeiterator.h"
#include <QObject>

class Scene : public QObject
{
    Q_OBJECT
public:
    Scene();
    virtual ~Scene();
    virtual void render();

    SceneNode* root() { return m_root; }

    SceneNodeIterator begin() const;

    void loadMesh(const QString &filename, glm::mat4 CTM=glm::mat4());

    void reset();

    void deleteSelectedNodes();
    void deleteSelectedMeshes();
    
    void roiSelectedParticles();
    void deroiSelectedParticles();

    void controlSelectedParticles();
    void decontrolSelectedParticles();

    void selectPriorGroup();
    void selectNextGroup();
    
    void clearSelection();
signals:
    void updateTool();
private:

    SceneNode* m_root;

    void setupLights();
};

#endif // SCENE_H
