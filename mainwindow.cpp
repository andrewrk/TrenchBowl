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
    timer->start();
}

MainWindow::~MainWindow()
{
    groove_destroy_player(player);
    delete ui;
}

void MainWindow::refreshToggleCaption()
{
    ui->toggleBtn->setText(groove_player_playing(player) ? "Pause" : "Play");
}


static QString fileDescription(GrooveFile *file) {
    GrooveTag *artist_tag = groove_file_metadata_get(file, "artist", NULL, 0);
    GrooveTag *title_tag = groove_file_metadata_get(file, "title", NULL, 0);

    if (artist_tag && title_tag) {
        QString artist = QString::fromUtf8(groove_tag_value(artist_tag));
        QString title = QString::fromUtf8(groove_tag_value(title_tag));
        return QString("%1 - %2").arg(artist, title);
    } else {
        QFileInfo info(file->filename);
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

static QString secondsDisplay(double seconds) {
    QTime time = QTime(0, 0, 0).addMSecs(seconds * 1000);
    const double ONE_HOUR = 60 * 60;
    QString fmt = seconds > ONE_HOUR ? "h:mm:ss" : "m:ss";
    return time.toString(fmt);
}

void MainWindow::refreshPosDisplay()
{
    double pos;
    GrooveQueueItem *item;
    groove_player_position(player, &item, &pos);
    if (!item) {
        ui->seekBar->setValue(ui->seekBar->minimum());
        ui->seekBar->setEnabled(false);
        ui->posLbl->setText("0:00");
        return;
    }
    double duration = groove_file_duration(item->file);
    double min = ui->seekBar->minimum();
    double max = ui->seekBar->maximum();
    double val = min + (max - min) * (pos / duration);
    ui->seekBar->setValue(val);
    ui->posLbl->setText(secondsDisplay(pos));
    ui->durationLbl->setText(secondsDisplay(duration));
    ui->seekBar->setEnabled(true);
}

void MainWindow::refreshNowPlaying() {
    refreshToggleCaption();
    refreshPosDisplay();

    double pos;
    GrooveQueueItem *item;
    groove_player_position(player, &item, &pos);

    if (item) {
        qDebug() << "item->prev" << item->prev;
        ui->prevBtn->setEnabled(item->prev != NULL);
        ui->nextBtn->setEnabled(item->next != NULL);
        QString desc = fileDescription(item->file);
        this->setWindowTitle(QString("%1 - TrenchBowl").arg(desc));
        ui->nowPlayingLbl->setText(desc);
        ui->durationLbl->setText(secondsDisplay(groove_file_duration(item->file)));
    } else {
        this->setWindowTitle("TrenchBowl");
        ui->nowPlayingLbl->setText("nothing playing");
        ui->durationLbl->setText("0:00");
        ui->prevBtn->setEnabled(false);
        ui->nextBtn->setEnabled(false);
    }

    ui->playlist->clear();
    GrooveQueueItem *node = player->queue_head;
    while (node) {
        ui->playlist->addItem(fileDescription(node->file));
        if (node == item) {
            QListWidgetItem *widget_item = ui->playlist->item(ui->playlist->count() - 1);
            QFont font = widget_item->font();
            font.setBold(true);
            widget_item->setFont(font);
        }
        node = node->next;
    }
}

void MainWindow::on_toggleBtn_clicked()
{
    if (groove_player_playing(player))
        groove_player_pause(player);
    else
        groove_player_play(player);
    refreshToggleCaption();
}

void MainWindow::on_nextBtn_clicked()
{
    GrooveQueueItem *item;
    double pos;
    groove_player_position(player, &item, &pos);
    if (item && item->next)
        groove_player_seek(player, item->next, 0);
}

void MainWindow::on_prevBtn_clicked()
{
    GrooveQueueItem *item;
    double pos;
    groove_player_position(player, &item, &pos);
    if (item && item->prev)
        groove_player_seek(player, item->prev, 0);
}
