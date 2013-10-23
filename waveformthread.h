#ifndef WAVEFORMTHREAD_H
#define WAVEFORMTHREAD_H

#include <QThread>

#include "waveformwidget.h"
#include "groove.h"

class WaveformThread : public QThread
{
    Q_OBJECT
public:
    explicit WaveformThread(WaveformWidget *waveform_widget, GrooveSink *sink, GroovePlayer *player, QObject *parent = 0);
    
    bool abort = false;
signals:
    
public slots:

private:
    WaveformWidget *waveform_widget;
    GrooveSink *sink;
    GroovePlayer *player;

    void run();
    
};

#endif // WAVEFORMTHREAD_H
