#include <QList>
#include <QStandardPaths>
#include <QDir>
#include "staticvariables.h"

bool StaticVariables::useFuzzySearch = true;
double StaticVariables::fuzzySearchThreshold = 41.0;

bool StaticVariables::ignoreTerminalApps = true;
bool StaticVariables::ignoreNoDisplayApps = true;
QString StaticVariables::defaultTerminal = "kitty";

QList<ResultItem> StaticVariables::defaultItems = QList<ResultItem>();

QStringList StaticVariables::targetDirs = QStandardPaths::locateAll(
    QStandardPaths::ApplicationsLocation, "" ,QStandardPaths::LocateDirectory);



bool StaticVariables::Save(){
    return false;
}
bool StaticVariables::Commit(){
    return false;
}
