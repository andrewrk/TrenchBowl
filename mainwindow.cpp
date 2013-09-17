#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QFileInfo>
#include <QtDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // start a player thread and listen to events
    groove_init();
    this->player = groove_create_player();
    this->player_thread = new PlayerThread(player, this);
    bool ok;
    ok = connect(this->player_thread, SIGNAL(nowPlayingUpdated()), this, SLOT(refreshNowPlaying()));
    Q_ASSERT(ok);
    ok = connect(this->ui->playlist, SIGNAL(queueSong(QString)), this, SLOT(queueSong(QString)));
    Q_ASSERT(ok);
    this->player_thread->start();
}

MainWindow::~MainWindow()
{
    groove_destroy_player(player);
    delete ui;
}

void MainWindow::refreshToggleCaption()
{
    switch (player->state) {
    case GROOVE_STATE_PAUSED:
    case GROOVE_STATE_STOPPED:
        ui->toggleBtn->setText("Play");
        break;
    case GROOVE_STATE_PLAYING:
        ui->toggleBtn->setText("Pause");
        break;
    }
}


static QString fileDescription(GrooveFile *file) {
    GrooveTag *artist_tag = groove_file_metadata_get(file, "artist", NULL, 0);
    GrooveTag *title_tag = groove_file_metadata_get(file, "title", NULL, 0);

    if (artist_tag && title_tag) {
        return QString("%1 - %2").arg(groove_tag_value(artist_tag), groove_tag_value(title_tag));
    } else {
        QFileInfo info(groove_file_filename(file));
        return info.fileName();
    }
}

void MainWindow::queueSong(QString song) {
    GrooveFile *file = groove_open(song.toUtf8().data());
    if (!file) {
        qDebug() << "Error opening" << song;
        return;
    }
    ui->playlist->addItem(fileDescription(file));
    groove_player_queue(player, file);
}

void MainWindow::refreshNowPlaying() {
    QString text = player->queue_head ? fileDescription(player->queue_head->file) : "nothing playing";
    this->ui->nowPlayingLbl->setText(text);
    refreshToggleCaption();
}

void MainWindow::on_toggleBtn_clicked()
{
    switch (player->state) {
    case GROOVE_STATE_PAUSED:
    case GROOVE_STATE_STOPPED:
        groove_player_play(player);
        break;
    case GROOVE_STATE_PLAYING:
        groove_player_pause(player);
        break;
    }
    refreshToggleCaption();
}
