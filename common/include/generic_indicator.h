#pragma once

#include <QScreen>
#include <QString>
#include <QWidget>

/**
 * @brief The GenericIndicator class is a QWidget that will pop up to inform the
 * user of something.
 */
class GenericIndicator : public QWidget {
    Q_OBJECT
  public:
    /**
     * @brief IndicatorPosition represents the position of the
     * indicator on your screen.
     */
    typedef enum {
        BOTTOM_RIGHT,
        BOTTOM_LEFT,
        TOP_RIGHT,
        TOP_LEFT,
        CENTER
    } IndicatorPosition;

    /**
     * @brief GenericIndicator constructor for this class.
     * @param note The string that will be displayed on the pop up.
     * @param location The position on the screen where the indicator
     * will pop up.
     * @param duration_ms is the amount of time in milliseconds the pop up
     * will be present on screen.
     */
    GenericIndicator(const QString &note, IndicatorPosition location,
                     int duration_ms = 1000);

  private:
    /**
     * @brief place_in_position Places the indicator in the desired location on
     * the screen
     * @param location The position that the indicator should be placed at.
     * @param screen A pointer to the screen that the indicator should be placed
     * on.
     */
    void place_in_position(IndicatorPosition location, QScreen *screen);
};
