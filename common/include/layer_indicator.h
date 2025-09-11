#ifndef LAYER_INDICATOR_H
#define LAYER_INDICATOR_H

#include <QString>
#include <QWidget>

/**
 * @brief The LayerIndicator class is a QWidget that will pop up when a user switches layers.
 * The purpose of this is to briefly tell the user what layer they are on, so that they
 * can orient themselves.
 */
class LayerIndicator : public QWidget
{
    //Q_OBJECT
  public:
    /**
     * @brief LayerIndicator constructor for this class.
     * @param layer_name The name of the layer that the user switched to.
     * This will be displayed in the pop up.
     * @param duration_ms is the amount of time in milliseconds the pop up
     * will be present on screen.
     */
    LayerIndicator(const QString& layer_name, int duration_ms);
};

#endif // LAYER_INDICATOR_H
