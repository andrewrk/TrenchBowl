#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <groove.h>
#include "playerthread.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

    GroovePlayer *player;
    PlayerThread *player_thread;

private slots:
    void refreshNowPlaying();
};

#endif // MAINWINDOW_H
