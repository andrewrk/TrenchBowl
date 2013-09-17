#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QTime>
#include <QtDebug>
#include <QTimer>
#include <QFont>

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
    ok = connect(this->ui->playlist, SIGNAL(queueUrl(QUrl)), this, SLOT(queueUrl(QUrl)));
    Q_ASSERT(ok);
    this->player_thread->start();

    QTimer *timer = new QTimer(this);
    timer->setInterval(16);
    ok = connect(timer, SIGNAL(timeout()), this, SLOT(refreshPosDisplay()));
    Q_ASSERT(ok);
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

void MainWindow::queueFile(QString file_path) {
    GrooveFile *file = groove_open(file_path.toUtf8().data());
    if (!file) {
        qDebug() << "Error opening" << file_path;
        return;
    }
    groove_player_queue(player, file);
    refreshNowPlaying();
}

void MainWindow::queueUrl(QUrl url) {
    if (url.isLocalFile()) {
        QString file_path = url.toLocalFile();
        QFileInfo file_path_info(file_path);
        if (file_path_info.isDir()) {
            QDirIterator iterator(QDir(file_path).absolutePath(), QDirIterator::Subdirectories);
            while (iterator.hasNext()) {
                iterator.next();
                if (iterator.fileInfo().isFile()) {
                    queueFile(iterator.filePath());
                }
            }
        } else {
            queueFile(file_path);
        }
    } else {
        queueFile(url.toString());
    }
}

void MainWindow::refreshPosDisplay()
{
}

static QString secondsDisplay(double seconds) {
    QTime time = QTime(0, 0, 0).addMSecs(seconds * 1000);
    const double ONE_HOUR = 60 * 60;
    QString fmt = seconds > ONE_HOUR ? "h:mm:ss" : "m:ss";
    return time.toString(fmt);
}

void MainWindow::refreshNowPlaying() {
    refreshToggleCaption();
    refreshPosDisplay();

    ui->prevBtn->setEnabled(false);
    if (player->queue_head) {
        GrooveFile *file = player->queue_head->file;
        QString desc = fileDescription(file);
        this->setWindowTitle(QString("%1 - TrenchBowl").arg(desc));
        ui->nowPlayingLbl->setText(desc);
        ui->nextBtn->setEnabled(true);
        ui->durationLbl->setText(secondsDisplay(groove_file_duration(file)));
    } else {
        this->setWindowTitle("TrenchBowl");
        ui->nowPlayingLbl->setText("nothing playing");
        ui->nextBtn->setEnabled(false);
        ui->durationLbl->setText("0:00");
    }

    ui->playlist->clear();
    GrooveQueueItem *item = player->queue_head;
    while (item) {
        ui->playlist->addItem(fileDescription(item->file));
        item = item->next;
    }
    QFont font = ui->playlist->item(0)->font();
    font.setBold(true);
    ui->playlist->item(0)->setFont(font);
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

void MainWindow::on_nextBtn_clicked()
{
    groove_player_next(player);
}
