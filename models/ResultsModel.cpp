#include "ResultsModel.h"
#include "static/staticvariables.h"
#include "static/searchfilters.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>
#include <QProcess>
#include <stdlib.h>
#include <QtConcurrent>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QClipboard>

ResultsModel::ResultsModel(QObject *parent) : QAbstractListModel(parent) {}
const QRegularExpression ResultsModel::desktopCodeRegex(R"(%[fFuUdDnNickvm])");

int ResultsModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    const auto &item = m_items[index.row()];

    switch (role) {
    case TitleRole:        return item.name;           // Using "name" for display title
    case NameRole:         return item.name;
    case GenericNameRole:  return item.genericName;
    case CommentRole:      return item.comment;
    case ExecRole:         return item.exec;
    case IconRole:         return item.icon;
    case TypeRole:         return item.type;
    case CategoriesRole:   return item.categories;
    case TerminalRole:     return item.terminal;
    case NoDisplayRole:    return item.noDisplay;
    case MimeTypeRole:     return item.mimeType;
    case StartupNotifyRole:return item.startupNotify;
    case HiddenRole:       return item.hidden;
    case KeywordsRole:     return item.keywords;
    case PathRole:         return item.path;
    case ScoreRole:        return item.score;
    case OriginRole:       return static_cast<int>(item.origin);;
    default:               return {};
    }
}
QString ResultsModel::getName(int index){
    if (m_items.isEmpty())
        return "";
    return m_items[index].name;
}
int ResultsModel::getOrigin(int index){
    if (m_items.isEmpty())
        return -1;
    return static_cast<int>(m_items[index].origin);
}


QHash<int, QByteArray> ResultsModel::roleNames() const {
    return {
        { TitleRole, "title" },
        { NameRole, "name" },
        { GenericNameRole, "genericName" },
        { CommentRole, "comment" },
        { ExecRole, "exec" },
        { IconRole, "icon" },
        { TypeRole, "type" },
        { CategoriesRole, "categories" },
        { TerminalRole, "terminal" },
        { NoDisplayRole, "noDisplay" },
        { MimeTypeRole, "mimeType" },
        { StartupNotifyRole, "startupNotify"},
        { HiddenRole, "hidden" },
        { KeywordsRole, "keywords" },
        { PathRole, "path"},
        { ScoreRole, "score"},
        { OriginRole, "origin"}
    };
}
void ResultsModel::clearItems(){
    if (m_items.isEmpty())
        return;
    beginResetModel();
    m_items.clear();
    endResetModel();
}
void ResultsModel::executeItem(const int index){
    if (m_items.isEmpty()) return;
    if (index == -1){
        ResultsModel::runApp(m_items[0]);
        return;
    }
    if (m_items.size() <= index) return;
    ResultsModel::runApp(m_items[index]);
}
void ResultsModel::setResults(const QList<ResultItem> &items)
{
    if (m_items == items)
        return;
    if (!m_items.isEmpty())
    {
        beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
        m_items.clear();
        endRemoveRows();
    }
    if (!items.isEmpty())
    {
        beginInsertRows(QModelIndex(), 0, items.size() - 1);
        m_items = items;
        endInsertRows();
    }
}
bool ResultsModel::getResults(){
    auto entries = ResultsModel::getDesktopEntries();
    if (entries.isEmpty())
        return false;
    m_allItems = entries;
    qDebug() << "Entries Found:" << entries.size();
    return true;
}

bool ResultsModel::searchResults(const QString &query){
    if (query.isEmpty())
        return false;

    QVector<ResultItem> results;
    results.reserve(m_allItems.size());

    SearchFilters::simpleSearch(results,query,m_allItems);
    SearchFilters::fuzzySearch(results,query.toLower().toStdString(),m_allItems);
    SearchFilters::googleSearch(results, query);
    SearchFilters::tryCalculate(results, query);

    if (results.isEmpty()) return false;

    std::sort(results.begin(), results.end(),
              [query](ResultItem &a, ResultItem &b) { return a.matchScore(query) > b.matchScore(query); });
    ResultsModel::setResults(results.toList());
    return true;
}

QList<ResultItem> ResultsModel::getDesktopEntries(){
    QStringList dirs = StaticVariables::targetDirs;
    return QtConcurrent::mappedReduced(dirs, [](const QString &dirString) -> QList<ResultItem>{
        QList<ResultItem> entries;
        auto dir = QDir(dirString);
        if (!dir.exists()) return entries;

        QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop", QDir::Files);
        for (const QString &fileName : desktopFiles) {
            QFile file(dir.filePath(fileName));
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

            ResultItem entry;
            entry.origin = ItemOrigin::ApplicationDirectory;
            QTextStream in(&file);
            bool inDesktopSection = false;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty() || line.startsWith('#')) continue;

                if (line == "[Desktop Entry]") {
                    inDesktopSection = true;
                    continue;
                }

                if (!inDesktopSection) continue;
                if (line.startsWith('[')) break;

                int idx = line.indexOf('=');
                if (idx == -1) continue;
                QString key = line.left(idx);
                QString value = line.mid(idx + 1);
                entry.setEntryField(key,value);
            }

            if (
                !entry.name.isEmpty() &&
                !entry.exec.isEmpty() &&
                !entry.icon.isEmpty() &&
                entry.terminal  != StaticVariables::ignoreTerminalApps &&
                entry.noDisplay != StaticVariables::ignoreNoDisplayApps) {

                entries.append(entry);
            }
            // else{
            //     qDebug() << "Ignored Entries:" << entry.name
            //              << "\nUnder Dir:" << dir.path()
            //              << "Name:" << entry.name
            //              << "Exec:" << entry.exec
            //              << "Icon:" << entry.icon;
            // }
            file.close();
        }
        return entries;
    },
    [](QList<ResultItem> &all, const QList<ResultItem> &part) {
        all.append(part);
    }).result();
}

void ResultsModel::runApp(const ResultItem &item){

    // setting Frequency
    if (item.origin == ItemOrigin::ApplicationDirectory){
        m_allItems[m_allItems.indexOf(item)].itemExecFrequency++;
    }

    qDebug() << "Executing..." << item.exec;
    qDebug()
        << "Entry:" << item.name
        << "Exec:" << item.exec
        << "Terminal:" << item.terminal
        << "No Display:" << item.noDisplay;

    if (item.origin == ItemOrigin::WebSearch){
        QDesktopServices::openUrl(QUrl(item.exec));
        return;
    }
    if (item.origin == ItemOrigin::Calculation){
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(item.name);
        return;
    }

    QString m_exec = item.exec;
    m_exec.replace(ResultsModel::desktopCodeRegex, "");
    m_exec = m_exec.simplified();


    if (item.terminal)
        QProcess::startDetached(StaticVariables::defaultTerminal, QStringList() << "-e" << m_exec);
    else
        QProcess::startDetached("sh", QStringList() << "-c" << m_exec);
}
