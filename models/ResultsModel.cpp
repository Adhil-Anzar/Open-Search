#include "ResultsModel.h"
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

#include <KIconLoader>

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
        { PathRole, "path"}
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
        ResultsModel::runApp(m_items[0].exec, m_items[0].terminal);
        return;
    }
    if (m_items.size() <= index) return;
    ResultsModel::runApp(m_items[index].exec, m_items[index].terminal);
}
void ResultsModel::setResults(const QList<ResultItem> &items) {
    if (m_items == items)
        return;
    // Creates a thread that does equating stuff!!
    QMetaObject::invokeMethod(this, [this, items]() {
        beginResetModel();
        m_items = items;
        endResetModel();
    }, Qt::QueuedConnection);
}
bool ResultsModel::getResults(){
    auto entries = ResultsModel::getDesktopEntries();
    if (entries.isEmpty())
        return false;
    // Creates a thread that does equating stuff!!
    m_allItems = entries;
    return true;
}

bool ResultsModel::searchResults(const QString &query){
    if (query.isEmpty())
        return false;

    const bool isSimpleSearch = false;
    const double threshold = 70;

    const std::string q = query.toLower().toStdString();
    QVector<ResultItem> results;
    results.reserve(m_allItems.size());
    if (isSimpleSearch){
        for (auto &it : m_allItems) {
            if (it.name.contains(query, Qt::CaseInsensitive)) {
                it.score = 100.0;
                results.append(it);
            }
        }
    }
    else {
        for (auto &it : m_allItems) {
            double score = rapidfuzz::fuzz::partial_ratio(q, it.name.toLower().toStdString()) * 0.7;
            score += std::max(score, rapidfuzz::fuzz::ratio(q, it.genericName.toLower().toStdString())) * 0.1;
            score += std::max(score, rapidfuzz::fuzz::ratio(q, it.comment.toLower().toStdString())) * 0.1;
            score += std::max(score, rapidfuzz::fuzz::ratio(q, it.keywords.toLower().toStdString())) * 0.1;
            if (it.type != QString("Application"))
                score -= 10;

            if (score > threshold) {
                it.score = score;
                results.append(it);
            }
        }
    }
    if (results.isEmpty())
        return false;

    std::sort(results.begin(), results.end(),
              [query](ResultItem &a, ResultItem &b) { return a.matchScore(query) > b.matchScore(query); });

    QMetaObject::invokeMethod(this, [this, results]() {
        beginRemoveRows(QModelIndex(), 0, m_items.size() - 1);
        m_items.clear();
        endRemoveRows();

        beginInsertRows(QModelIndex(), 0, results.size() - 1);
        for (const auto &si : results) {
            m_items.append(si);
        }
        endInsertRows();
    }, Qt::QueuedConnection);
    return true;
}

QList<ResultItem> ResultsModel::getDesktopEntries(){
    QList<ResultItem> entries;

    QStringList dirs = {
        "/usr/share/applications/",
        QDir::homePath() + "/.local/share/flatpak/exports/share/applications/",
        "/var/lib/flatpak/exports/share/applications/",
        QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)
    };

    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop", QDir::Files);
        for (const QString &fileName : desktopFiles) {
            QFile file(dir.filePath(fileName));
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

            ResultItem entry;
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

                if (key == "Name") entry.name = value;
                else if (key == "GenericName") entry.genericName = value;
                else if (key == "Comment") entry.comment = value;
                else if (key == "Exec") entry.exec = value;
                else if (key == "Icon"){
                    entry.icon = KIconLoader::global()->iconPath(value, KIconLoader::Desktop);
                    if (entry.icon.isEmpty()){
                        entry.icon = KIconLoader::global()->iconPath(value, KIconLoader::Any);
                        if (entry.icon.isEmpty()){
                            entry.icon = KIconLoader::global()->iconPath("preferences-desktop-display", KIconLoader::Any);
                        }
                    }
                }
                else if (key == "Type") entry.type = value;
                else if (key == "Categories") entry.categories = value;
                else if (key == "Terminal") entry.terminal = (value.toLower() == "true");
                else if (key == "NoDisplay") entry.noDisplay = (value.toLower() == "true");
                else if (key == "MimeType") entry.mimeType = value;
                else if (key == "StartupNotify") entry.startupNotify = value;
                else if (key == "Hidden") entry.hidden = value;
                else if (key == "Keywords") entry.keywords = value;
                else if (key == "Path") entry.path = value;
            }

            if (!entry.name.isEmpty() && !entry.exec.isEmpty() && !entry.icon.isEmpty()) {
                entries.append(entry);
            }
            else{
                qDebug() << "Ignored Entries:" << entry.name
                         << "\nUnder Dir:" << dir.path()
                         << "Name:" << entry.name
                         << "Exec:" << entry.exec
                         << "Icon:" << entry.icon;
            }
            file.close();
        }
    }
    return entries;
}

void ResultsModel::runApp(const QString &exec, bool terminal){

    qDebug() << "exec: " << exec << "list: ";
    QString m_exec = exec;
    m_exec.replace(ResultsModel::desktopCodeRegex, "");
    m_exec = m_exec.simplified();/*

    QStringList parts = QProcess::splitCommand(m_exec);
    QString program = parts.takeFirst();

    qDebug() << "Program: " << program << " Part :" << parts;
    if (parts.isEmpty())
        QProcess::startDetached(program);
    else
        QProcess::startDetached(program, parts);*/
    if (terminal)
        QProcess::startDetached("kitty", QStringList() << "-e" << m_exec);
    else
        QProcess::startDetached("sh", QStringList() << "-c" << m_exec);
}
