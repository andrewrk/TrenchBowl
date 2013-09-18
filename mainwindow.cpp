#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QTime>
#include <QtDebug>
#include <QTimer>
#include <QFont>
#include <QSlider>

static void setSliderDouble(QSlider *slider, double value) {
    double min = slider->minimum();
    double max = slider->maximum();
    slider->setValue(min + (max - min) * value);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // start a player thread and listen to events
    groove_init();
    groove_set_logging(GROOVE_LOG_INFO);
    this->player = groove_create_player();
    this->player_thread = new PlayerThread(player, this);
    bool ok;
    ok = connect(this->player_thread, SIGNAL(nowPlayingUpdated()), this, SLOT(refreshNowPlaying()));
    Q_ASSERT(ok);
    ok = connect(this->ui->playlist, SIGNAL(queueUrl(QUrl)), this, SLOT(queueUrl(QUrl)));
    Q_ASSERT(ok);
    ok = connect(this->ui->playlist, SIGNAL(deletePressed()), this, SLOT(removeSelectedItems()));
    Q_ASSERT(ok);
    this->player_thread->start();

    QTimer *timer = new QTimer(this);
    timer->setInterval(16);
    ok = connect(timer, SIGNAL(timeout()), this, SLOT(refreshPosDisplay()));
    Q_ASSERT(ok);
    timer->start();

    double rg_default = groove_player_get_replaygain_default(player);
    double rg_preamp = groove_player_get_replaygain_preamp(player);
    double vol = groove_player_get_volume(player);

    setSliderDouble(ui->volSlider, vol);
    setSliderDouble(ui->preampSlider, rg_preamp);
    setSliderDouble(ui->defaultSlider, rg_default);

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
    groove_player_insert(player, file, NULL);
    refreshNowPlaying();
}

void MainWindow::setSelectedRgMode(GrooveReplayGainMode mode)
{
    foreach(QListWidgetItem *item, ui->playlist->selectedItems()) {
        GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
        groove_player_set_replaygain_mode(player, q_item, mode);
    }
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
    if (seek_down)
        return;

    double pos;
    GroovePlaylistItem *item;
    groove_player_position(player, &item, &pos);
    if (!item) {
        ui->seekBar->setValue(ui->seekBar->minimum());
        ui->seekBar->setEnabled(false);
        ui->posLbl->setText("0:00");
        return;
    }
    double duration = groove_file_duration(item->file);
    setSliderDouble(ui->seekBar, pos / duration);
    ui->posLbl->setText(secondsDisplay(pos));
    ui->durationLbl->setText(secondsDisplay(duration));
    ui->seekBar->setEnabled(true);
}

void MainWindow::removeSelectedItems()
{
    foreach (QListWidgetItem *item, ui->playlist->selectedItems()) {
        GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
        groove_player_remove(player, q_item);
    }
    qDeleteAll(ui->playlist->selectedItems());
}

void MainWindow::refreshNowPlaying() {
    refreshToggleCaption();
    refreshPosDisplay();

    GroovePlaylistItem *item;
    groove_player_position(player, &item, NULL);

    if (item) {
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
    GroovePlaylistItem *node = player->playlist_head;
    while (node) {
        ui->playlist->addItem(fileDescription(node->file));
        QListWidgetItem *widget_item = ui->playlist->item(ui->playlist->count() - 1);
        if (node == item) {
            QFont font = widget_item->font();
            font.setBold(true);
            widget_item->setFont(font);
        }
        widget_item->setData(Qt::UserRole, qVariantFromValue((void *) node));

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
    GroovePlaylistItem *item;
    groove_player_position(player, &item, NULL);
    if (item && item->next)
        groove_player_seek(player, item->next, 0);
}

void MainWindow::on_prevBtn_clicked()
{
    GroovePlaylistItem *item;
    groove_player_position(player, &item, NULL);
    if (item && item->prev)
        groove_player_seek(player, item->prev, 0);
}

void MainWindow::on_seekBar_sliderPressed()
{
    groove_player_position(player, &seek_down, NULL);
}

void MainWindow::on_seekBar_sliderReleased()
{
    seek_down = NULL;
}

void MainWindow::on_seekBar_sliderMoved(int position)
{
    if (!seek_down)
        return;

    double duration = groove_file_duration(seek_down->file);
    double min = ui->seekBar->minimum();
    double max = ui->seekBar->maximum();
    double pos = duration * ((position - min) / (max - min));

    ui->posLbl->setText(secondsDisplay(pos));
    groove_player_seek(player, seek_down, pos);
}

void MainWindow::on_playlist_itemDoubleClicked(QListWidgetItem *item)
{
    GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
    groove_player_seek(player, q_item, 0);
}

void MainWindow::on_preampSlider_sliderMoved(int position)
{
    double min = ui->preampSlider->minimum();
    double max = ui->preampSlider->maximum();
    double val = (position - min) / (max - min);
    groove_player_set_replaygain_preamp(player, val);
}

void MainWindow::on_defaultSlider_sliderMoved(int position)
{
    double min = ui->preampSlider->minimum();
    double max = ui->preampSlider->maximum();
    double val = (position - min) / (max - min);
    groove_player_set_replaygain_default(player, val);
}

void MainWindow::on_volSlider_sliderMoved(int position)
{
    double min = ui->volSlider->minimum();
    double max = ui->volSlider->maximum();
    double val = (position - min) / (max - min);
    groove_player_set_volume(player, val);
}

void MainWindow::on_playlist_itemClicked(QListWidgetItem *item)
{
    GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
    switch (q_item->replaygain_mode) {
    case GROOVE_REPLAYGAINMODE_OFF:
        ui->optRgOff->setChecked(true);
        break;
    case GROOVE_REPLAYGAINMODE_ALBUM:
        ui->optRgAlbum->setChecked(true);
        break;
    case GROOVE_REPLAYGAINMODE_TRACK:
        ui->optRgTrack->setChecked(true);
        break;
    }
    GrooveTag *album_rg_tag = groove_file_metadata_get(q_item->file, "REPLAYGAIN_ALBUM_GAIN", NULL, 0);
    GrooveTag *track_rg_tag = groove_file_metadata_get(q_item->file, "REPLAYGAIN_TRACK_GAIN", NULL, 0);
    QString album_rg = "(missing)";
    QString track_rg = "(missing)";
    if (album_rg_tag)
        album_rg = QString::fromUtf8(groove_tag_value(album_rg_tag));
    if (track_rg_tag)
        track_rg = QString::fromUtf8(groove_tag_value(track_rg_tag));
    ui->optRgAlbum->setText(QString("Album: %1").arg(album_rg));
    ui->optRgTrack->setText(QString("Track: %1").arg(track_rg));

}

void MainWindow::on_optRgOff_clicked()
{
    setSelectedRgMode(GROOVE_REPLAYGAINMODE_OFF);
}

void MainWindow::on_optRgAlbum_clicked()
{

    setSelectedRgMode(GROOVE_REPLAYGAINMODE_ALBUM);
}

void MainWindow::on_optRgTrack_clicked()
{

    setSelectedRgMode(GROOVE_REPLAYGAINMODE_TRACK);
}
