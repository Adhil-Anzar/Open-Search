#include "searchfilters.h"
#include <QUrl>

void SearchFilters::simpleSearch(QVector<ResultItem> &results, const QString &query, QList<ResultItem> &m_allItems){
    if (StaticVariables::useFuzzySearch) return;
    for (auto &it : m_allItems) {
        if (it.name.contains(query, Qt::CaseInsensitive)) {
            it.score = 100.0;
            results.append(it);
        }
    }
}
void SearchFilters::fuzzySearch(QVector<ResultItem> &results, const std::string &query, QList<ResultItem> &m_allItems){
    if (!StaticVariables::useFuzzySearch) return;
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
void SearchFilters::googleSearch(QVector<ResultItem> &results, const QString &query){
    if (!StaticVariables::useGoogleSearch) return;
    const double threshold = StaticVariables::fuzzySearchThreshold;
    const double score = qMin(
        threshold + 10.0,
        100.0 * (1.0 - std::exp(
                    - StaticVariables::googleSearchCurveMultiplier * qMax(0, query.length()
                    - StaticVariables::googleSearchInitCount)))
        );
    ResultItem item;
    item.name = "Search Google";
    item.icon = "fcitx-googlepinyin-panel";
    item.comment = QStringLiteral("Search Google for \"%1\"").arg(query);
    item.exec = QStringLiteral("https://www.google.com/search?q=%1")
                    .arg(QUrl::toPercentEncoding(query));
    item.score = score;
    item.terminal = false;
    item.origin = ItemOrigin::WebSearch;

    results.append(item);
}
