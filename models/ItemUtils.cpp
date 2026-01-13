#include "models/ItemUtils.h"

ItemUtils::ItemUtils(QObject *parent) : QObject(parent) {}

QString ItemUtils::resolveIconPath(const QString &iconName)
{
    // Check cache first
    if (m_iconCache.contains(iconName))
        return m_iconCache.value(iconName);

    // Try Desktop theme first
    QString path = KIconLoader::global()->iconPath(iconName, KIconLoader::Desktop);

    // Fallback to Any theme
    if (path.isEmpty())
        path = KIconLoader::global()->iconPath(iconName, KIconLoader::Any);

    // Last resort: use a default generic icon
    if (path.isEmpty())
        path = KIconLoader::global()->iconPath("preferences-desktop-display", KIconLoader::Any);

    // Store in cache
    m_iconCache.insert(iconName, path);

    return path;
}
