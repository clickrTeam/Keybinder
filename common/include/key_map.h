#ifndef KEY_MAP_H
#define KEY_MAP_H
#include <QHash>
#include <QString>

class KeyMap
{
  public:
    KeyMap();
    // Insert a key string and its corresponding keycode
    void insert(const QString& name, int key_code);
    // Remove a key string and its corresponding keycode
    void remove(const QString& name, int key_code);
    // Lookup by string key (e.g., "a" -> KEY_A)
    int string_to_key_code(const QString& name) const;
    // Lookup by keycode (e.g., KEY_A -> "a")
    QString key_code_to_string(int keyCode) const;
    // Check if name exists
    bool contains_string(const QString& name) const;
    // Check if keycode exists
    bool contains_key_code(int keyCode) const;



  private:
    QHash<QString, int> string_to_key_map;
    QHash<int, QString> key_to_string_map;
    QList<QPair<QString, int>> create_list();
};

#endif // KEY_MAP_H
