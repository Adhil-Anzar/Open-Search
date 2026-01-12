#include <QString>
#include <QImage>

struct ResultItem {
    QString name;           // Name= in .desktop
    QString genericName;    // GenericName=
    QString comment;        // Comment=
    QString exec;           // Exec=
    QString icon;           // Icon=
    QString type;           // Type=
    QString categories;     // Categories=
    bool terminal = false;  // Terminal=true/false
    bool noDisplay = false; // NoDisplay=true/false
    QString mimeType;       // MimeType=
    QString startupNotify;  // StartupNotify=
    QString hidden;         // Hidden=
    QString keywords;       // Keywords=
    QString path;           // Path=, optional
    double score;           // For the count of times this gets used up

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
};
