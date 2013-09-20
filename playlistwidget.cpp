#include "playlistwidget.h"

#include <QtDebug>
#include <QDragEnterEvent>
#include <QUrl>
#include <QMimeData>


PlaylistWidget::PlaylistWidget(QWidget *parent) :
    QListWidget(parent)
{
}


void PlaylistWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void PlaylistWidget::dropEvent(QDropEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    if (!event->mimeData()->hasUrls())
        return;

    QList<QUrl> urls = event->mimeData()->urls();
    foreach (QUrl url, urls) {
        emit queueUrl(url.toString());
    }
}

void PlaylistWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void PlaylistWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        emit deletePressed();
        event->accept();
    }
}
