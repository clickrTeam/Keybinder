#ifndef KEYMAP_H
#define KEYMAP_H

#include <QHash>
#include <QString>

class KeyMap
{
    public:
        KeyMap();
        // Insert a key string and its corresponding keycode
        void insert(const QString& name, int keyCode);
        // Lookup by string key (e.g., "a" -> KEY_A)
        int getKey(const QString& name) const;
        // Lookup by keycode (e.g., KEY_A -> "a")
        QString getName(int keyCode) const;
        // Check if name exists
        bool containsName(const QString& name) const;
        // Check if keycode exists
        bool containsKey(int keyCode) const;

    private:
        QHash<QString, int> strToKey;
        QHash<int, QString> keyToStr;
};
#endif // KEYMAP_H
