#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QListWidget>

class PlaylistWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);
    

protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;

signals:
    void queueSong(QString);
    
public slots:
    
};

#endif // PLAYLISTWIDGET_H
