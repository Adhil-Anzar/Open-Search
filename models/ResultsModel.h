#pragma once
#include <QAbstractListModel>
#include "structs/ResultItem.cpp"

class ResultsModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        NameRole,
        GenericNameRole,
        CommentRole,
        ExecRole,
        IconRole,
        TypeRole,
        CategoriesRole,
        TerminalRole,
        NoDisplayRole,
        MimeTypeRole,
        StartupNotifyRole,
        HiddenRole,
        KeywordsRole,
        PathRole,
    };

    explicit ResultsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;


    Q_INVOKABLE bool searchResults(const QString &searchQuery = {});
    Q_INVOKABLE void clearItems();
    Q_INVOKABLE void executeItem(const int index);

    void setResults(const QList<ResultItem> &items);
    bool getResults();

private:
    void runApp(const QString &exec, bool terminal);
    QList<ResultItem> getDesktopEntries();
    static const QRegularExpression desktopCodeRegex;
    QList<ResultItem> m_items;
    QList<ResultItem> m_allItems;
};
