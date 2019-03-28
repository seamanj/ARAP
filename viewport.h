#ifndef VIEWPORT_H
#define VIEWPORT_H

#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include <QRectF>
class Camera;
class Viewport
{

public:

    enum State
    {
        IDLE,
        PANNING,
        ZOOMING,
        TUMBLING
    };

    Viewport();
    ~Viewport();

    Camera* getCamera() const { return m_camera; }

    void loadMatrices() const;
    void loadPickMatrices( const glm::ivec2 &click, float size ) const;
    void loadPickMatrices( const QRectF &rect) const;
    static void popMatrices();

    void push() const;
    void pop() const;

    void orient( const glm::vec3 &eye, const glm::vec3 &lookAt, const glm::vec3 &up );

    void setDimensions( int width, int height );

    void setState( State state ) { m_state = state; }
    State getState() const { return m_state; }

    void mouseMoved();
    void drawAxis();

private:

    State m_state;
    Camera *m_camera;
    int m_width, m_height;


};

#endif // VIEWPORT_H
