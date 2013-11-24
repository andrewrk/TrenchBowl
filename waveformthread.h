#ifndef WAVEFORMTHREAD_H
#define WAVEFORMTHREAD_H

#include <QThread>

#include "waveformwidget.h"
#include <grooveplayer/player.h>

class WaveformThread : public QThread
{
    Q_OBJECT
public:
    explicit WaveformThread(WaveformWidget *waveform_widget, GrooveSink *sink, GroovePlayer *player, QObject *parent = 0);
    
    bool abort = false;

    static void flush(GrooveSink *sink);
signals:
    
public slots:

private:
    WaveformWidget *waveform_widget;
    GrooveSink *sink;
    GroovePlayer *player;

    bool flush_flag = false;

    void run();
    
};

#endif // WAVEFORMTHREAD_H
