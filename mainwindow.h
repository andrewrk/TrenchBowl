#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

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

    void refreshToggleCaption();
    void queueFile(QString file_path);

private slots:
    void refreshNowPlaying();
    void queueUrl(QUrl song);
    void refreshPosDisplay();

    void on_toggleBtn_clicked();
    void on_nextBtn_clicked();
    void on_prevBtn_clicked();
};

#endif // MAINWINDOW_H
