
//#include <Windows.h>
#include <QFileDialog>
#include <qevent.h>
#include <QString>

#include <assert.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tool.h"
#include "uisettings.h"
#include "databinding.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    UiSettings::loadSettings();
    ui->setupUi(this);
    setupUI();

    this->setWindowTitle( "ARAP" );
    this->move( UiSettings::windowPosition() );
    this->resize( UiSettings::windowSize() );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startSimulation()
{
    if ( ui->viewPanel->startSimulation() ) {
        ui->viewPanel->clearSelection();
        ui->selectionToolButton->click();
//        ui->startButton->setEnabled( false );
//        ui->stopButton->setEnabled( true );
//        ui->pauseButton->setEnabled( true );
//        ui->resetButton->setEnabled( false );
    }
}

void MainWindow::stopSimulation()
{
    ui->viewPanel->stopSimulation();
//    ui->startButton->setEnabled( true );
//    ui->stopButton->setEnabled( false );
//    if ( ui->pauseButton->isChecked() ) {
//        ui->pauseButton->click();
//    }
//    ui->pauseButton->setEnabled( false );
//    ui->resetButton->setEnabled( true );
}

void MainWindow::setupUI()
{
    assert( connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(importMesh())));
    assert( connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(exportMesh())));
    
    // partilize
    
    assert( connect(ui->particlizeButton, SIGNAL(clicked()), ui->viewPanel, SLOT(particlize())));
    assert( connect(ui->ROIButton, SIGNAL(clicked()), ui->viewPanel, SLOT(roiSelectedParticles())));
    assert( connect(ui->deROIButtron, SIGNAL(clicked()), ui->viewPanel, SLOT(deroiSelectedParticles())));
    assert( connect(ui->controlButton, SIGNAL(clicked()), ui->viewPanel, SLOT(controlSelectedParticles())));
    assert( connect(ui->deControlButton, SIGNAL(clicked()), ui->viewPanel, SLOT(decontrolSelectedParticles())));
    assert( connect(ui->priorGroupButton, SIGNAL(clicked()), ui->viewPanel, SLOT(selectPriorGroup())));
    assert( connect(ui->nextGroupButton, SIGNAL(clicked()), ui->viewPanel, SLOT(selectNextGroup())));
    
//    ComboIntAttribute::bindSlot( ui->deformationCombo, ui->viewPanel, SLOT(deformationModeChanged(int))  );
//    assert( connect(ui->viewPanel, SIGNAL(changeSelection(int)), ui->deformationCombo, SLOT(setCurrentIndex(int))));
    
    // simulation
    
//    assert( connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startSimulation())) );
//    assert( connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopSimulation())) );
//    assert( connect(ui->pauseButton, SIGNAL(toggled(bool)), ui->viewPanel, SLOT(pauseSimulation(bool))) );
//    assert( connect(ui->resetButton, SIGNAL(clicked()), ui->viewPanel, SLOT(resetSimulation())) );
    
    // ViewPanel
    
    ComboIntAttribute::bindInt( ui->showMeshesCombo, &UiSettings::showMeshesMode(), this );
    ComboIntAttribute::bindInt( ui->showParticleSystemsCombo, &UiSettings::showParticleSystemsMode(), this );
    
    BoolBinding::bindCheckBox( ui->meshCheckBox, UiSettings::showMesh(), this );
    BoolBinding::bindCheckBox( ui->particleSystemCheckBox, UiSettings::showPariticleSystem(), this );
    BoolBinding::bindCheckBox( ui->particleCheckBox, UiSettings::showParticle(), this );
    
    // Tools
    ui->toolButtonGroup->setId( ui->selectionToolButton, Tool::SELECTION );
    ui->toolButtonGroup->setId( ui->moveToolButton, Tool::MOVE );
    ui->toolButtonGroup->setId( ui->rotateToolButton, Tool::ROTATE );
    ui->toolButtonGroup->setId( ui->scaleToolButton, Tool::SCALE );
    assert( connect(ui->toolButtonGroup, SIGNAL(buttonClicked(int)), ui->viewPanel, SLOT(setTool(int))) );
    ui->selectionToolButton->click();
}

void MainWindow::importMesh()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select off to import.", PROJECT_PATH, "*.off");
    if ( !filename.isEmpty() ) {
        ui->viewPanel->loadMesh( filename );
    }
}
void MainWindow::exportMesh()
{
    QString filename = QFileDialog::getSaveFileName( this, "Choose mesh file destination.", PROJECT_PATH );
    if ( !filename.isEmpty() ) {
        ui->viewPanel->saveMesh( filename );
    }

}

void MainWindow::keyPressEvent( QKeyEvent *event )
{
    ui->viewPanel->keyPressEvent(event);
    if ( event->key() == Qt::Key_Q ) {
        ui->selectionToolButton->click();
        event->accept();
    }
   else if ( event->key() == Qt::Key_W ) {
        ui->moveToolButton->click();
        event->accept();
    } else if ( event->key() == Qt::Key_E ) {
        ui->rotateToolButton->click();
        event->accept();
    } else if ( event->key() == Qt::Key_R ) {
        ui->scaleToolButton->click();
        event->accept();
    }
    else {
        event->setAccepted( false );
    }
}
