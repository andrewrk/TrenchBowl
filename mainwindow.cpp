#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QFileInfo>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // start a player thread and listen to events
    groove_init();
    this->player = groove_create_player();
    this->player_thread = new PlayerThread(player, this);
    bool ok = connect(this->player_thread, SIGNAL(nowPlayingUpdated()), this, SLOT(refreshNowPlaying()));
    Q_ASSERT(ok);
    this->player_thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshNowPlaying() {
    QString text;
    if (this->player->queue_head) {
        GrooveFile *file = this->player->queue_head->file;
        GrooveTag *artist_tag = groove_file_metadata_get(file, "artist", NULL, 0);
        GrooveTag *title_tag = groove_file_metadata_get(file, "title", NULL, 0);

        if (artist_tag && title_tag) {
            text = QString("%1 - %2").arg(groove_tag_value(artist_tag), groove_tag_value(title_tag));
        } else {
            QFileInfo info(groove_file_filename(file));
            text = info.fileName();
        }
    } else {
        text = "nothing playing";
    }
    this->ui->nowPlayingLbl->setText(text);
}
