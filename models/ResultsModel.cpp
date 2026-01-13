#include "ResultsModel.h"
#include "static/staticvariables.h"
#include <rapidfuzz/fuzz.hpp>

#include <QDir>
#include <QIcon>
#include <QPixmap>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <stdlib.h>
#include <QtConcurrent>

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
    default:               return {};
    }
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
        { ScoreRole, "score"}
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
void ResultsModel::setResults(const QList<ResultItem> &items) {
    if (m_items == items)
        return;
    // Creates a thread that does equating stuff!!
    // QMetaObject::invokeMethod(this, [this, items]() {
    //     beginResetModel();
    //     m_items = items;
    //     endResetModel();
    // }, Qt::QueuedConnection);
    const int size = items.size();
    if (size == 0){
        clearItems();
    }
    beginResetModel();
    m_items = items;
    endResetModel();
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
    const bool isSimpleSearch = StaticVariables::useFuzzySearch;

    if (!StaticVariables::useFuzzySearch) ResultsModel::beginSimpleSearch(results,query);
    else ResultsModel::beginFuzzySearch(results,query.toLower().toStdString());

    if (results.isEmpty()) return false;

    std::sort(results.begin(), results.end(),
              [query](ResultItem &a, ResultItem &b) { return a.matchScore(query) > b.matchScore(query); });

    // QMetaObject::invokeMethod(this, [this, results]() {
    //     beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
    //     m_items.clear();
    //     endRemoveRows();

    //     beginInsertRows(QModelIndex(), 0, results.size() - 1);
    //     for (const auto &si : results) {
    //         m_items.append(si);
    //     }
    //     endInsertRows();
    // }, Qt::QueuedConnection);
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
    m_allItems[m_allItems.indexOf(item)].itemExecFrequency++;

    qDebug() << "Executing..." << item.exec;
    qDebug()
        << "Ignored Entries:" << item.name
        << "Exec:" << item.exec
        << "Terminal:" << item.terminal
        << "Display:" << item.noDisplay;

    QString m_exec = item.exec;
    m_exec.replace(ResultsModel::desktopCodeRegex, "");
    m_exec = m_exec.simplified();

    if (item.terminal)
        QProcess::startDetached(StaticVariables::defaultTerminal, QStringList() << "-e" << m_exec);
    else
        QProcess::startDetached("sh", QStringList() << "-c" << m_exec);
}
void ResultsModel::beginSimpleSearch(QVector<ResultItem> &results, const QString &query){
    for (auto &it : m_allItems) {
        if (it.name.contains(query, Qt::CaseInsensitive)) {
            it.score = 100.0;
            results.append(it);
        }
    }
}
void ResultsModel::beginFuzzySearch(QVector<ResultItem> &results, const std::string &query){
    const double threshold = StaticVariables::fuzzySearchThreshold;
    for (auto &it : m_allItems) {
        double score = rapidfuzz::fuzz::partial_ratio(query, it.name.toLower().toStdString()) * 0.7;
        score += rapidfuzz::fuzz::ratio(query, it.genericName.toLower().toStdString()) * 0.1;
        score += rapidfuzz::fuzz::ratio(query, it.comment.toLower().toStdString()) * 0.1;
        score += rapidfuzz::fuzz::ratio(query, it.keywords.toLower().toStdString()) * 0.1;
        if (it.type != QString("Application"))
            score -= 10;

        if (score > threshold) {
            it.score = score;
            results.append(it);
        }
    }
}
