#include "selectiontool.h"

#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif

#include "userinput.h"

#include "scenenode.h"
#include "viewpanel.h"
#include "picker.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <QDebug>
#include <memory>
using std::dynamic_pointer_cast;

SelectionTool::SelectionTool( ViewPanel *panel ,Type t)
    : Tool(panel,t)
{
}

SelectionTool::~SelectionTool()
{
}
void
SelectionTool::mousePressed()
{
    m_pressedPos = UserInput::mousePos();
    Tool::mousePressed();
    m_selectionRect = QRectF(0, 0, 0, 0);
}
void SelectionTool::render()
{
    if( m_mouseDown)
    {
        glPushAttrib( GL_LIGHTING_BIT );
        glDisable( GL_LIGHTING );
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, m_viewPanelSize.width(), m_viewPanelSize.height(), 0, -1,1);      
        //qDebug() << m_viewPanelSize.width() << "," << m_viewPanelSize.height();
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(m_selectionRect.left(), m_selectionRect.top());
            glVertex2f(m_selectionRect.right(), m_selectionRect.top());
            glVertex2f(m_selectionRect.right(), m_selectionRect.bottom());
            glVertex2f(m_selectionRect.left(), m_selectionRect.bottom());
        glEnd();
        //glRectf(m_selectionRect.left(), m_selectionRect.top(), m_selectionRect.right(), m_selectionRect.bottom());
        //qDebug() << m_selectionRect.left() << m_selectionRect.top() << m_selectionRect.right() << m_selectionRect.bottom();
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glPopAttrib();
    }
}
//void SelectionTool::updateSize(int width, int height)
//{
//    //qDebug() << width << "," << height;
//    m_viewPanelSize = QSizeF(width, height);
//}
void SelectionTool::mouseMoved()
{
    if( m_mouseDown )
    {
        glm::vec2 curPos = UserInput::mousePos();
        m_selectionRect = QRectF(m_pressedPos.x, m_pressedPos.y, curPos.x - m_pressedPos.x, curPos.y - m_pressedPos.y);
    }
}

void
SelectionTool::mouseReleased()
{
    if ( m_mouseDown ) {
        
        if(m_selectionRect == QRectF(0, 0, 0, 0) )
        {
            if( !UserInput::shiftKey() )
                clearSelection();
            SceneNode *selected = getSelectedSceneNode();
            if( selected &&  selected->isSelectable())
            {
                if ( UserInput::shiftKey() ) {
                    if ( selected ) selected->getRenderable()->setSelected( !selected->getRenderable()->isSelected() );
                } else {
                    //clearSelection();
                    if ( selected ) selected->getRenderable()->setSelected( true );
                }

            }

        }
        else if( (m_selectionRect.right() - m_selectionRect.left()) != 0 && (m_selectionRect.bottom() - m_selectionRect.top()) != 0)
        {
            if( !UserInput::shiftKey() )
                clearSelection();
            QList<SceneNode*> selected = getSelectedSceneNodes();
            for(int i = 0; i < selected.size(); ++i)
            {
                if(selected[i] && selected[i]->isSelectable())
                {
                    if ( UserInput::shiftKey() ) {
                        if ( selected[i] ) selected[i]->getRenderable()->setSelected( !selected[i]->getRenderable()->isSelected() );
                    } else {
                        //clearSelection();
                        if ( selected[i] ) selected[i]->getRenderable()->setSelected( true );
                    }
                }

            }

        }
        Tool::mouseReleased();
    }
    m_panel->checkSelected();
}
void
SelectionTool::clearSelection()
{
//    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
//        if ( (*it)->hasRenderable() ) {
//            (*it)->getRenderable()->setSelected( false );
//        }
//    }
    m_panel->clearSelection();
}

SceneNode*
SelectionTool::getSelectedSceneNode()
{
    m_panel->m_viewport->loadPickMatrices( UserInput::mousePos(), 3.f );
    SceneNode * clicked = NULL;

    QList<SceneNode*> renderables;
    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() ) {
            renderables += (*it);
        }
    }
    if ( !renderables.empty() ) {
        Picker picker( renderables.size() );
        for ( int i = 0; i < renderables.size(); ++i ) {
            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
            glMultMatrixf( glm::value_ptr(renderables[i]->getCTM()) );
            picker.setObjectIndex(i);
            renderables[i]->getRenderable()->renderForPicker();
            glPopMatrix();
        }
        unsigned int index = picker.getPick();
        
        if ( index != Picker::NO_PICK ) {
            clicked = renderables[index];
        }
    }
    
    m_panel->m_viewport->popMatrices();
    return clicked;
}

QList<SceneNode*>
SelectionTool::getSelectedSceneNodes()
{
    //    if(m_selectionRect == QRectF(0, 0, 0, 0) )
    //        m_panel->m_viewport->loadPickMatrices( UserInput::mousePos(), 3.f );
    //    else
    //qDebug() << m_selectionRect.left() << m_selectionRect.top() << m_selectionRect.right() << m_selectionRect.bottom();
    m_panel->m_viewport->loadPickMatrices(m_selectionRect);
    
    QList<SceneNode *> clicked;
    
    QList<SceneNode*> renderables;
    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() ) {
            renderables += (*it);
        }
    }
    if ( !renderables.empty() ) {
        Picker picker( renderables.size() );
        for ( int i = 0; i < renderables.size(); ++i ) {
            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
            glMultMatrixf( glm::value_ptr(renderables[i]->getCTM()) );
            picker.setObjectIndex(i);
            renderables[i]->getRenderable()->renderForPicker();
            glPopMatrix();
        }
        QList<unsigned int> indices = picker.getPicks();
        for( int i = 0; i < indices.size(); ++i)
        {
            clicked += renderables[indices[i]];
        }
    }

    m_panel->m_viewport->popMatrices();
    return clicked;
}
bool
SelectionTool::hasSelection( glm::vec3 &center ) const
{
    center = glm::vec3( 0, 0, 0 );
    int count = 0;
    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() && (*it)->getRenderable()->isSelected() &&
             (*it)->isSelectable()) {
            center += (*it)->getCentroid();
            count++;
        }
    }
    center /= (float)count;
    //std::cout << center.x << center.y << center.z << std::endl;
    return ( count > 0 );
}
bool
SelectionTool::hasMovableSelection( glm::vec3 &center ) const
{
    center = glm::vec3( 0, 0, 0 );
    int count = 0;
    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() && (*it)->getRenderable()->isSelected() &&
             (*it)->isMovable() ) 
        {
            if( (*it)->isParticle() )
            {
                if( dynamic_pointer_cast<Particle>((*it)->getRenderable())->isControl() )
                {
                    center += (*it)->getRenderable()->getCentroid();
                    count++;
                }
            }
            else
            {
                center += (*it)->getCentroid();
                count++;
            }
        }
    }
    center /= (float)count;
    return ( count > 0 );
}

bool
SelectionTool::hasRotatableSelection( glm::vec3 &center ) const
{
    center = glm::vec3( 0, 0, 0 );
    int count = 0;
    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() && (*it)->getRenderable()->isSelected() &&
             (*it)->isRotatable() ) {
            center += (*it)->getCentroid();
            count++;
        }
    }
    center /= (float)count;
    return ( count > 0 );
}
bool
SelectionTool::hasScalableSelection( glm::vec3 &center ) const
{
    center = glm::vec3( 0, 0, 0 );
    int count = 0;
    for ( SceneNodeIterator it = m_panel->m_scene->begin(); it.isValid(); ++it ) {
        if ( (*it)->hasRenderable() && (*it)->getRenderable()->isSelected() &&
             (*it)->isScable() ) {
            center += (*it)->getCentroid();
            count++;
        }
    }
    center /= (float)count;
    return ( count > 0 );
}
