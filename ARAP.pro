#-------------------------------------------------
#
# Project created by QtCreator 2016-01-04T15:08:40
#
#-------------------------------------------------

QT       += core gui opengl xml

CONFIG   += console


# OpenGL stuff
LIBS += -lGLEW -lCGAL
# -lCGAL_Core
#-lCGAL -lCGAL_Core
DEFINES += GL_GLEXT_PROTOTYPES
DEFINES += BOOST_ALL_DYN_LINK
DEFINES += CGAL_EIGEN3_ENABLED 
#DEFINES += LD_LIBRARY_PATH=/usr/local/lib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ARAP
TEMPLATE = app

DEFINES += PROJECT_PATH=\\\"$$_PRO_FILE_PWD_\\\"
DEFINES += GLM_ENABLE_EXPERIMENTAL


QMAKE_CXXFLAGS_RELEASE=-O3
QMAKE_CXXFLAGS += -std=c++11

#INCLUDEPATH += "c:/work_files/glm"
#INCLUDEPATH += "c:\work_files\OPENGL\include"
#INCLUDEPATH += "c:\work_files\boost_1_55_0"
INCLUDEPATH += "/home/seamanj/Software/eigen-eigen-323c052e1731"
#INCLUDEPATH += "c:\work_files\CGAL-4.7\auxiliary\gmp\include"
#INCLUDEPATH += "c:\work_files\CGAL-4.7\build\include"
#INCLUDEPATH += "c:\work_files\CGAL-4.7\build_vs2015_x64\include"
#
#
#QMAKE_LIBDIR += "/home/seamanj/Software/cgal/build/lib"
#QMAKE_LIBDIR += "/home/seamanj/Software/cgal/build/lib"
#QMAKE_LIBDIR += "c:\work_files\CGAL-4.7\build\lib"
#QMAKE_LIBDIR += "c:\work_files\boost_1_55_0\stage\lib"

#INCLUDEPATH += "/opt/glm"
#INCLUDEPATH += "d:\work_files\GL\include"
#INCLUDEPATH += "d:\work_files\boost_1_55_0"
#INCLUDEPATH += "/opt/eigen-eigen-c58038c56923"
#INCLUDEPATH += "d:\work_files\CGAL-4.7\include"
#INCLUDEPATH += "d:\work_files\CGAL-4.7\auxiliary\gmp\include"
#INCLUDEPATH += "d:\work_files\CGAL-4.7\build\include"


#QMAKE_LIBDIR += "d:\work_files\GL\lib\Win32"
#QMAKE_LIBDIR += "d:\work_files\CGAL-4.7\build\lib"
#QMAKE_LIBDIR += "d:\work_files\boost_1_55_0\stage\lib"


SOURCES += main.cpp\
        mainwindow.cpp \
    viewpanel.cpp \
    objparser.cpp \
    mesh.cpp \
    scenenode.cpp \
    scene.cpp \
    viewport.cpp \
    userinput.cpp \
    tool.cpp \
    selectiontool.cpp \
    movetool.cpp \
    picker.cpp \
    rotatetool.cpp \
    scaletool.cpp \
    infopanel.cpp \
    uisettings.cpp \
    particle.cpp \
    engine.cpp \
    particlesystem.cpp

HEADERS  += mainwindow.h \
    viewpanel.h \
    objparser.h \
    types.h \
    renderable.h \
    mesh.h \
    common.h \
    scenenode.h \
    scenenodeiterator.h \
    scene.h \
    viewport.h \
    camera.h \
    userinput.h \
    tool.h \
    selectiontool.h \
    movetool.h \
    picker.h \
    mymath.h \
    rotatetool.h \
    scaletool.h \
    infopanel.h \
    uisettings.h \
    databinding.h \
    particle.h \
    engine.h \
    particlesystem.h \
    graphfunction.h \
    mypolyhedron.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons/icons.qrc
