#include "layer_indicator.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

LayerIndicator::LayerIndicator(const QString &layer_name,
                               int duration_ms) {
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
    resize(sizeHint());  // ensures width() and height() are valid

    // Place the indicator in the bottom right corner of the screen where the
    // mouse is.
    QPoint cursor_pos = QCursor::pos();
    QScreen *screen = QApplication::screenAt(cursor_pos);

    if (!screen) {
        qDebug() << "Cursor position not found, defaulting to primary screen";
        screen = QApplication::primaryScreen();
    }

    QRect screen_geometry = screen->availableGeometry();
    QSize logicalSize = screen->geometry().size();
    QSize physSize = screen->size();  // physical in mm, not pixels
    qDebug() << "Logical:" << logicalSize
            << "Reported DPR:" << screen->devicePixelRatio()
            << "Pixel density:" << screen->logicalDotsPerInchX();
    int x = screen_geometry.right() - width() - 20;
    int y = screen_geometry.bottom() - height() - 40;
    qDebug() << "Screen geom:" << screen_geometry 
         << "Widget:" << width() << "x" << height()
         << "Target:" << x << "," << y;


    show();
    move(x, y);

    // Auto-close after duration
    QTimer::singleShot(duration_ms, this, &QWidget::close);
}
