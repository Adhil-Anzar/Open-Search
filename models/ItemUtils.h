#pragma once
#include <KIconLoader>
#include <QHash>

class ItemUtils : public QObject
{
    Q_OBJECT
public:
    explicit ItemUtils(QObject *parent = nullptr);
    Q_INVOKABLE QString resolveIconPath(const QString &iconName);
private:
    QHash<QString, QString> m_iconCache;
};
