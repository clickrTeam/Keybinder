#include "layer_indicator.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>


LayerIndicator::LayerIndicator(const QString &layer_name,
                               int duration_ms) {
    qDebug() << "Layer Indicator constructor for: " << layer_name;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);

    auto *label = new QLabel(layer_name);
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

    // Compute global target coordinates using availableGeometry() (logical pixels)
    QRect screen_geom = screen->availableGeometry();
    const int margin_right = 20;
    const int margin_bottom = 40;
    int x = screen_geom.x() + screen_geom.width() - width() - margin_right;
    int y = screen_geom.y() + screen_geom.height() - height() - margin_bottom;

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

    // Debug logging to verify mapping
    qDebug() << "Cursor:" << cursor_pos
             << "Screen:" << (screen ? screen->name() : QString("<none>"))
             << "ScreenGeom:" << screen_geom
             << "Widget pos after show:" << mapToGlobal(QPoint(0,0))
             << "DPR:" << (screen ? screen->devicePixelRatio() : 1.0);

    // Auto-close after duration
    QTimer::singleShot(duration_ms, this, &QWidget::close);
}
