#include "searchfilters.h"
#include <QUrl>
#include "exprtk/exprtk.hpp"

bool SearchFilters::useFuzzySearch = true;
double SearchFilters::fuzzySearchThreshold = 57.0;

bool SearchFilters::useGoogleSearch = true;
int SearchFilters::googleSearchInitCount = 8;
double SearchFilters::googleSearchCurveMultiplier = 0.5;

bool SearchFilters::useCalculation = true;

void SearchFilters::simpleSearch(QVector<ResultItem> &results, const QString &query, QList<ResultItem> &m_allItems){
    if (SearchFilters::useFuzzySearch) return;
    for (auto &it : m_allItems) {
        if (it.name.contains(query, Qt::CaseInsensitive)) {
            it.score = 100.0;
            results.append(it);
        }
    }
}
void SearchFilters::fuzzySearch(QVector<ResultItem> &results, const std::string &query, QList<ResultItem> &m_allItems){
    if (!SearchFilters::useFuzzySearch) return;
    const double threshold = SearchFilters::fuzzySearchThreshold;
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
    if (!SearchFilters::useGoogleSearch) return;
    const double threshold = SearchFilters::fuzzySearchThreshold;
    const double score = qMin(
        threshold + 10.0,
        100.0 * (1.0 - std::exp(
                    - SearchFilters::googleSearchCurveMultiplier * qMax(0, query.length()
                    - SearchFilters::googleSearchInitCount)))
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
void SearchFilters::tryCalculate(QVector<ResultItem> &results, const QString &query){
    if (!SearchFilters::useCalculation) return;
    double result;

    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double> expression_t;
    typedef exprtk::parser<double> parser_t;

    expression_t expression;
    symbol_table_t symbol_table;
    expression.register_symbol_table(symbol_table);

    parser_t parser;
    if (parser.compile(query.toStdString(), expression))
        result = expression.value();
    else
        return;
    ResultItem item;
    item.name = QString::number(result);
    item.icon = "accessories-calculator";
    item.comment = QStringLiteral("Calculated for: \"%1\"").arg(query);
    item.exec = "Ignored";
    item.score = 100.0;
    item.terminal = false;
    item.origin = ItemOrigin::Calculation;

    results.append(item);
}
