#include "waveformthread.h"

WaveformThread::WaveformThread(WaveformWidget *waveform_widget, GrooveSink *sink, GroovePlayer *player, QObject *parent) :
    QThread(parent),
    waveform_widget(waveform_widget),
    sink(sink),
    player(player)
{
}

void WaveformThread::flush(GrooveSink *sink)
{
    WaveformThread *waveform_thread = reinterpret_cast<WaveformThread*>(sink->userdata);
    waveform_thread->flush_flag = true;
}

void WaveformThread::run()
{
    GrooveBuffer *buffer;

    GroovePlaylistItem *waveform_item = NULL;
    double waveform_pos = 0;

    while (!abort) {
        GroovePlaylistItem *player_item;
        double player_pos;
        groove_player_position(player, &player_item, &player_pos);
        if (!flush_flag && (waveform_item != player_item || waveform_pos > player_pos)) {
            // waveform is ahead; sleep
            QThread::msleep(5);
            continue;
        }
        flush_flag = false;
        int result = groove_sink_buffer_get(sink, &buffer, 1);
        if (result == GROOVE_BUFFER_YES) {
            waveform_item = buffer->item;
            waveform_pos = buffer->pos;

            waveform_widget->processAudio(buffer);
        } else if (result < 0 || result == GROOVE_BUFFER_NO) {
             break;
        }
    }
}
