#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:

    void importMesh();
    void exportMesh();
    void startSimulation();
    void stopSimulation();
    virtual void keyPressEvent( QKeyEvent *event );

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    void setupUI();
};

#endif // MAINWINDOW_H
