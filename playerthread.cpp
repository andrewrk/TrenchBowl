#include "playerthread.h"


PlayerThread::PlayerThread(GroovePlayer *player, QObject *parent) :
    QThread(parent),
    player(player)
{
}

void PlayerThread::run() {
    GrooveEvent event;
    while (groove_player_event_get(player, &event, 1) >= 0) {
        switch (event.type) {
        case GROOVE_EVENT_BUFFERUNDERRUN:
            break;
        case GROOVE_EVENT_NOWPLAYING:
            emit nowPlayingUpdated();
            break;
        }
    }
}
