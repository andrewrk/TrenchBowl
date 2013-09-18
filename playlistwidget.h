#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QUrl>

class PlaylistWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);
    

protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;

    void keyPressEvent(QKeyEvent *event) override;

signals:
    void queueUrl(QUrl);
    void deletePressed();
    
public slots:
    
};

#endif // PLAYLISTWIDGET_H
