#include "waveformwidget.h"

#include <cfloat>
#include <QPainter>
#include <QMutexLocker>
#include <QtDebug>

WaveformWidget::WaveformWidget(QWidget *parent) :
    QWidget(parent)
{
}

void WaveformWidget::processAudio(GrooveBuffer *new_buffer)
{
    QMutexLocker(&this->mutex);

    groove_buffer_unref(buffer);
    buffer = new_buffer;

    this->update();
}

void WaveformWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.eraseRect(this->rect());


    QMutexLocker(&this->mutex);

    if (!buffer) return;



    // get duration of the buffer
    int channel_count = groove_channel_layout_count(buffer->format.channel_layout);
    int frames_per_pixel = buffer->frame_count / this->width();
    int samples_per_pixel = frames_per_pixel * channel_count;
    double channel_count_mult = 1 / (double)channel_count;
    int image_bound_y = this->height() - 1;

    double *buf = reinterpret_cast<double*>(buffer->data[0]);

    painter.setPen(Qt::blue);

    int sample_index = 0;
    for (int x = 0; x < this->width(); x += 1) {
        double min = 1.0;
        double max = -1.0;
        for (int i = 0; i < samples_per_pixel; i += channel_count) {
            // average the channels
            double value = 0;
            for (int c = 0; c < channel_count; c += 1) {
                double sample = buf[sample_index];
                value += sample * channel_count_mult;
                sample_index += 1;
            }
            // keep track of max/min
            if (value < min) min = value;
            if (value > max) max = value;
        }
        // translate into y pixel coord
        int y_min = (min + 1.0) * image_bound_y / 2.0;
        int y_max = (max + 1.0) * image_bound_y / 2.0;

        painter.drawLine(x, y_min, x, y_max);

    }
}
