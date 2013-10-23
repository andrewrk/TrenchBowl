#include "waveformwidget.h"

#include <cfloat>
#include <QPainter>
#include <QMutexLocker>

WaveformWidget::WaveformWidget(QWidget *parent) :
    QWidget(parent)
{
}

void WaveformWidget::processAudio(GrooveBuffer *new_buffer)
{
    QMutexLocker(&this->mutex);

    groove_buffer_unref(buffer);
    buffer = new_buffer;
}

void WaveformWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.eraseRect(this->rect());

    if (!buffer) return;


    QMutexLocker(&this->mutex);

    // get duration of the buffer
    int channel_count = groove_channel_layout_count(buffer->format.channel_layout);
    int sample_size = groove_sample_format_bytes_per_sample(buffer->format.sample_fmt);
    int frame_size = channel_count * sample_size;
    int frames_per_pixel = buffer->frame_count / this->width();
    int frames_times_channels = frames_per_pixel * channel_count;
    double channel_count_mult = 1 / (double)channel_count;
    int image_bound_y = this->height() - 1;

    painter.setPen(Qt::black);

    int frame_index = 0;
    for (int x = 0; x < this->width(); x += 1) {
        double min = DBL_MAX;
        double max = DBL_MIN;
        for (int i = 0; i < frames_times_channels; i += channel_count) {
            // average the channels
            double value = 0;
            for (int c = 0; c < channel_count; c += 1) {
                int offset = frame_index * frame_size + c * sample_size;
                double *sample = reinterpret_cast<double*>(buffer->data[0] + offset);
                value += *sample * channel_count_mult;
            }
            // keep track of max/min
            if (value < min) min = value;
            if (value > max) max = value;
            frame_index += 1;
        }
        // translate into y pixel coord
        int y_min = min / DBL_MAX * image_bound_y;
        int y_max = max / DBL_MAX * image_bound_y;
        painter.drawLine(x, y_min, x, y_max);

    }
}
