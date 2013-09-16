#include "playerthread.h"


PlayerThread::PlayerThread(GroovePlayer *player, QObject *parent) :
    QThread(parent),
    player(player)
{
}

void PlayerThread::run() {
    GroovePlayerEvent event;
    while (groove_player_event_wait(player, &event) >= 0) {
        switch (event.type) {
        case GROOVE_PLAYER_EVENT_NOWPLAYING:
            emit nowPlayingUpdated();
            break;
        }
    }
}
