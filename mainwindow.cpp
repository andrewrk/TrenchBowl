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
    atexit(groove_finish);
    groove_set_logging(GROOVE_LOG_INFO);
    this->playlist = groove_playlist_create();
    this->player = groove_player_create();
    this->waveform_sink = groove_sink_create();
    this->waveform_sink->audio_format.channel_layout = GROOVE_CH_LAYOUT_STEREO;
    this->waveform_sink->audio_format.sample_fmt = GROOVE_SAMPLE_FMT_DBL;
    this->waveform_sink->audio_format.sample_rate = 44100;
    this->player_thread = new PlayerThread(this->player, this);
    this->waveform_thread = new WaveformThread(this->ui->waveformWidget, this->waveform_sink, this->player, this);
    this->waveform_sink->userdata = this->waveform_thread;
    this->waveform_sink->flush = WaveformThread::flush;
    this->waveform_thread->start();
    groove_player_attach(this->player, this->playlist);
    groove_sink_attach(this->waveform_sink, this->playlist);
    bool ok;
    Q_UNUSED(ok);
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

    setSliderDouble(ui->volSlider, playlist->volume);
    setSliderDouble(ui->preampSlider, rg_preamp);
    setSliderDouble(ui->defaultSlider, rg_default);
}

MainWindow::~MainWindow()
{
    waveform_thread->abort = true;
    waveform_thread->wait();
    groove_player_detach(player);
    groove_sink_detach(waveform_sink);
    groove_player_destroy(player);
    groove_playlist_destroy(playlist);
    delete ui;
}

void MainWindow::refreshToggleCaption()
{
    ui->toggleBtn->setText(groove_playlist_playing(playlist) ? "Pause" : "Play");
}


static QString fileDescription(GrooveFile *file) {
    GrooveTag *artist_tag = groove_file_metadata_get(file, "artist", NULL, 0);
    GrooveTag *title_tag = groove_file_metadata_get(file, "title", NULL, 0);

    if (artist_tag && title_tag) {
        QString artist = QString::fromUtf8(groove_tag_value(artist_tag));
        QString title = QString::fromUtf8(groove_tag_value(title_tag));
        return QString("%1 - %2").arg(artist, title);
    } else {
        QFileInfo info(QString::fromUtf8(file->filename));
        return info.fileName();
    }
}

void MainWindow::queueFile(QString file_path) {
    GrooveFile *file = groove_file_open(file_path.toUtf8().data());
    if (!file) {
        qDebug() << "Error opening" << file_path;
        return;
    }
    groove_playlist_insert(playlist, file, 1.0, NULL);
    refreshNowPlaying();
}

void MainWindow::setSelectedRgMode(ReplayGainMode )
{
    foreach(QListWidgetItem *item, ui->playlist->selectedItems()) {
        GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
        // TODO adjust gain based on replaygain
        groove_playlist_set_gain(playlist, q_item, 1.0);
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
        groove_playlist_remove(playlist, q_item);
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
    GroovePlaylistItem *node = playlist->head;
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
    if (groove_playlist_playing(playlist))
        groove_playlist_pause(playlist);
    else
        groove_playlist_play(playlist);
    refreshToggleCaption();
}

void MainWindow::on_nextBtn_clicked()
{
    GroovePlaylistItem *item;
    groove_player_position(player, &item, NULL);
    if (item && item->next)
        groove_playlist_seek(playlist, item->next, 0);
}

void MainWindow::on_prevBtn_clicked()
{
    GroovePlaylistItem *item;
    groove_player_position(player, &item, NULL);
    if (item && item->prev)
        groove_playlist_seek(playlist, item->prev, 0);
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
    groove_playlist_seek(playlist, seek_down, pos);
}

void MainWindow::on_playlist_itemDoubleClicked(QListWidgetItem *item)
{
    GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
    groove_playlist_seek(playlist, q_item, 0);
}

void MainWindow::on_preampSlider_sliderMoved(int position)
{
    double min = ui->preampSlider->minimum();
    double max = ui->preampSlider->maximum();
    double val = (position - min) / (max - min);
    rg_preamp = val;
    // TODO update gain on the item
}

void MainWindow::on_defaultSlider_sliderMoved(int position)
{
    double min = ui->preampSlider->minimum();
    double max = ui->preampSlider->maximum();
    double val = (position - min) / (max - min);
    rg_default = val;
    // TODO update gain on the item
}

void MainWindow::on_volSlider_sliderMoved(int position)
{
    double min = ui->volSlider->minimum();
    double max = ui->volSlider->maximum();
    double val = (position - min) / (max - min);
    groove_playlist_set_volume(playlist, val);
}

void MainWindow::on_playlist_itemClicked(QListWidgetItem *item)
{
    GroovePlaylistItem *q_item = (GroovePlaylistItem *)item->data(Qt::UserRole).value<void *>();
    switch (replaygain_mode) {
    case REPLAYGAIN_OFF:
        ui->optRgOff->setChecked(true);
        break;
    case REPLAYGAIN_ALBUM:
        ui->optRgAlbum->setChecked(true);
        break;
    case REPLAYGAIN_TRACK:
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
    setSelectedRgMode(REPLAYGAIN_OFF);
}

void MainWindow::on_optRgAlbum_clicked()
{

    setSelectedRgMode(REPLAYGAIN_ALBUM);
}

void MainWindow::on_optRgTrack_clicked()
{

    setSelectedRgMode(REPLAYGAIN_TRACK);
}
