#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QListWidgetItem>

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

    GroovePlaylist *playlist;
    GroovePlayer *player;
    PlayerThread *player_thread;

    GroovePlaylistItem *seek_down = NULL;


    double rg_default = 0.25;
    double rg_preamp = 0.75;

    enum ReplayGainMode {
        REPLAYGAIN_OFF,
        REPLAYGAIN_TRACK,
        REPLAYGAIN_ALBUM,
    };

    ReplayGainMode replaygain_mode = REPLAYGAIN_OFF;

    void refreshToggleCaption();
    void queueFile(QString file_path);
    void setSelectedRgMode(ReplayGainMode mode);

private slots:
    void refreshNowPlaying();
    void queueUrl(QUrl song);
    void refreshPosDisplay();
    void removeSelectedItems();

    void on_toggleBtn_clicked();
    void on_nextBtn_clicked();
    void on_prevBtn_clicked();
    void on_seekBar_sliderPressed();
    void on_seekBar_sliderReleased();
    void on_seekBar_sliderMoved(int position);
    void on_playlist_itemDoubleClicked(QListWidgetItem *item);
    void on_preampSlider_sliderMoved(int position);
    void on_defaultSlider_sliderMoved(int position);
    void on_volSlider_sliderMoved(int position);
    void on_playlist_itemClicked(QListWidgetItem *item);
    void on_optRgOff_clicked();
    void on_optRgAlbum_clicked();
    void on_optRgTrack_clicked();
};

#endif // MAINWINDOW_H
