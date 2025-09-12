#include "layer_indicator.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

LayerIndicator::LayerIndicator(const QString &layer_name,
                               int duration_ms = 1000) {
    qDebug() << "Layer Indicator constructor for: " << layer_name;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                   Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);

    auto *label = new QLabel(layer_name);
    label->setStyleSheet(
        "QLabel { color: white; background-color: rgba(0, 0, 0, 180); padding: "
        "10px; border-radius: 5px; font-size: 14pt; }");
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->setContentsMargins(0, 0, 0, 0);
    adjustSize();

    // Place the indicator in the bottom right corner of the screen where the
    // mouse is.
    QPoint cursor_pos = QCursor::pos();
    QScreen *screen = QApplication::screenAt(cursor_pos);

    if (!screen) {
        qDebug() << "Cursor position not found, defaulting to primary screen";
        screen = QApplication::primaryScreen();
    }

    QRect screen_geometry = screen->availableGeometry();
    int x = screen_geometry.right() - width() - 20;
    int y = screen_geometry.bottom() - height() - 40;
    move(x, y);

    show();

    // Auto-close after duration
    QTimer::singleShot(duration_ms, this, &QWidget::close);
}
