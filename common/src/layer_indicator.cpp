#include "layer_indicator.h"
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QVBoxLayout>
#include <QDebug>

LayerIndicator::LayerIndicator(const QString& layer_name, int duration_ms = 1000)
{
    qDebug() << "Layer Indicator constructor for: " << layer_name;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    auto* label = new QLabel(layer_name);
    label->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 180); padding: 10px; border-radius: 5px; font-size: 14pt; }");
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->setContentsMargins(0, 0, 0, 0);
    adjustSize();

    // Place the indicator in the bottom right corner of the primary screen.
    QRect primary_screen_geometry = QGuiApplication::primaryScreen()->geometry();
    int x = primary_screen_geometry.width() - width() - 20;
    int y = primary_screen_geometry.height() - height() - 40;
    move(x, y);

    show();

           // Auto-close after duration
    QTimer::singleShot(duration_ms, this, &QWidget::close);
}
