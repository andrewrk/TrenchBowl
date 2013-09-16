#ifndef PLAYERTHREAD_H
#define PLAYERTHREAD_H

#include <QThread>

#include "groove.h"

class PlayerThread : public QThread
{
    Q_OBJECT
public:
    explicit PlayerThread(GroovePlayer *player, QObject *parent = 0);

signals:
    void nowPlayingUpdated();
private:
    GroovePlayer *player;
    void run();
};


#endif // PLAYERTHREAD_H
