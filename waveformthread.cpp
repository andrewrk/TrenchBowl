#include "waveformthread.h"

WaveformThread::WaveformThread(WaveformWidget *waveform_widget, GrooveSink *sink, QObject *parent) :
    QThread(parent),
    waveform_widget(waveform_widget),
    sink(sink)
{
}

void WaveformThread::run()
{
    GrooveBuffer *buffer;
    for (;;) {
        int result = groove_sink_get_buffer(sink, &buffer, 1);
        if (result == GROOVE_BUFFER_YES) {
            waveform_widget->processAudio(buffer);
            waveform_widget->update();
        } else if (result < 0 || result == GROOVE_BUFFER_NO) {
             break;
        }
    }
}
