#pragma once
#include <QString>
#include <QMap>
#pragma once
#include <KIconLoader>
#include "enums/ItemOrigin.cpp"
#include "structs/ShortResultItem.h"
#include "static/staticvariables.h"
struct ResultItem {
    QString name;           // Name= in .desktop
    QString genericName;    // GenericName=
    QString comment;        // Comment=
    QString exec;           // Exec=
    QString icon;           // Icon=
    QString type;           // Type=
    QString categories;     // Categories=
    QString mimeType;       // MimeType=
    QString startupNotify;  // StartupNotify=
    QString hidden;         // Hidden=
    QString keywords;       // Keywords=
    QString path;           // Path=, optional
    ItemOrigin origin;      // To check what to exec on runApp
    double score;           // For the count of times this gets used up
    int itemExecFrequency;   // The number of times the gets executed
    bool terminal = false;  // Terminal=true/false
    bool noDisplay = false; // NoDisplay=true/false


    const static QMap<QString, std::function<void(ResultItem&, const QString&)>> entryFieldHandlers;

    bool operator==(const ResultItem& other) const
    {
        return name == other.name && exec == other.exec;
    }
    int matchScore(const QString &query) const {
        if (name.startsWith(query, Qt::CaseInsensitive))
            return 1000 + score;

        if (genericName.startsWith(query, Qt::CaseInsensitive))
            return 900 + score;

        if (name.contains(query, Qt::CaseInsensitive))
            return 500 + score;

        if (genericName.contains(query, Qt::CaseInsensitive))
            return 400 + score;

        if (keywords.contains(query, Qt::CaseInsensitive))
            return 300 + score;

        return score;
    }
    ShortResultItem toShortResultItem(){
        ShortResultItem r;
        r.name = name;
        r.exec = exec;
        r.terminal = terminal;
        r.itemExecFrequency = itemExecFrequency;
        return r;
    }
    void setEntryField(const QString &key, const QString &value) {
        auto handlers = entryFieldHandlers;
        auto it = handlers.find(key);
        if (it != handlers.end())
            it.value()(*this, value);
    }

};

