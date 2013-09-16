#include "playlistwidget.h"

#include <QtDebug>


PlaylistWidget::PlaylistWidget(QWidget *parent) :
    QListWidget(parent)
{
}


void PlaylistWidget::dragEnterEvent(QDragEnterEvent *)
{
    qDebug() << "dragEnterEvent";
}

void PlaylistWidget::dropEvent(QDropEvent *)
{
    qDebug() << "dropEvent";
}
