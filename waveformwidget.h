#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QMutex>

#include "groove.h"

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget *parent = 0);

    // assumes sample format double, 44100 Hz, stereo, interleaved
    // ok to call from non-main thread
    void processAudio(GrooveBuffer *new_buffer);

protected:
    void paintEvent(QPaintEvent *) override;

private:

    GrooveBuffer *buffer = NULL;

    // how many seconds of audio the widget represents
    double waveform_length = 1.0;

    QMutex mutex;

};

#endif // WAVEFORMWIDGET_H
