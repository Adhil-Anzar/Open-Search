#include <QList>
#include <QStandardPaths>
#include <QDir>
#include "staticvariables.h"

bool StaticVariables::useFuzzySearch = false;
double StaticVariables::fuzzySearchThreshold = 70.0;

QList<ResultItem> StaticVariables::defaultItems = QList<ResultItem>();
QString StaticVariables::defaultTerminal = "kitty";

QStringList StaticVariables::targetDirs = QStandardPaths::locateAll(
    QStandardPaths::ApplicationsLocation, "",QStandardPaths::LocateDirectory);

bool StaticVariables::Save(){
    return false;
}
bool StaticVariables::Commit(){
    return false;
}
