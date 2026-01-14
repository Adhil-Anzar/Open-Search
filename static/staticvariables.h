#pragma once
#include <QList>
#include "structs/ResultItem.h"

class StaticVariables
{
public:
    static bool ignoreTerminalApps;
    static bool ignoreNoDisplayApps;
    static QString defaultTerminal;
    static QList<ResultItem> defaultItems;

    static QStringList targetDirs;

    static bool Save();
    static bool Commit();

private:
    StaticVariables() = delete;
    ~StaticVariables() = delete;
};
