#include "generic_indicator.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>


GenericIndicator::GenericIndicator(const QString &note, IndicatorPosition location,
                               int duration_ms) {
    qDebug() << "Generic Indicator constructor for: " << note;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);

    auto *label = new QLabel(note);
    label->setStyleSheet(
        "QLabel { color: white; background-color: rgba(0, 0, 0, 180); padding: 10px; "
        "border-radius: 5px; font-size: 14pt; }");
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->setContentsMargins(0, 0, 0, 0);

    // Ensure layout is calculated and widget metrics are ready
    layout->activate();
    ensurePolished();
    adjustSize();
    resize(sizeHint());

    // Find the screen under the cursor, fallback to a screen containing the cursor
    QPoint cursor_pos = QCursor::pos();
    QScreen *screen = QApplication::screenAt(cursor_pos);
    if (!screen) {
        // Iterate through all screens
        for (QScreen *s : QApplication::screens()) {
            if (s->geometry().contains(cursor_pos)) {
                screen = s;
                break;
            }
        }
    }
    if (!screen) {
        // No screen found via cursor position, default to primary screen
        screen = QApplication::primaryScreen();
    }

    place_in_position(location, screen);
    // Auto-close after duration
    QTimer::singleShot(duration_ms, this, &QWidget::close);
}

void GenericIndicator::place_in_position(IndicatorPosition location, QScreen *screen) {
    if (screen) {
        // Compute global target coordinates using availableGeometry() (logical pixels)
        QRect screen_geom = screen->availableGeometry();
        const int margin_horizontal = 20;
        const int margin_vertical = 40;
        int x = 0;
        int y = 0;

        switch (location) {
            case BOTTOM_RIGHT: {
                x = screen_geom.x() + screen_geom.width() - width() - margin_horizontal;
                y = screen_geom.y() + screen_geom.height() - height() - margin_vertical;
                break;
            }
            case BOTTOM_LEFT: {
                x = screen_geom.x() + margin_horizontal;
                y = screen_geom.y() + screen_geom.height() - height() - margin_vertical;
                break;
            }
            case TOP_RIGHT: {
                x = screen_geom.x() + screen_geom.width() - width() - margin_horizontal;
                y = screen_geom.y() + margin_vertical;
                break;
            }
            case TOP_LEFT: {
                x = screen_geom.x() + margin_horizontal;
                y = screen_geom.y() + margin_vertical;
                break;
            }
            case CENTER: {
                x = screen_geom.x() + (screen_geom.width() - width()) / 2;
                y = screen_geom.y() + (screen_geom.height() - height()) / 2;
                break;
            }
            default: {
                qCritical() << "Unexpected screen location in Generic Indicator";
                break;
            }
        }
        // Ensure the native QWindow exists and set its screen before showing
        if (!windowHandle()) {
            createWinId(); // forces native handle creation on platforms where needed
        }
        if (QWindow *wh = windowHandle()) {
            wh->setScreen(screen);
        }

        // Move the widget in global coordinates before showing to avoid compositor placement
        move(x, y);
        show();
    }
    else {
        qCritical() << "Attempted to calculate indicator position on a null pointer screen.";
    }
}
