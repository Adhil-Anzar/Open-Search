#pragma once
#include "structs/ResultItem.h"
#include <rapidfuzz/fuzz.hpp>

class SearchFilters
{
public:
    static bool useFuzzySearch;
    static double fuzzySearchThreshold;

    static bool useGoogleSearch;
    static int googleSearchInitCount;
    static double googleSearchCurveMultiplier;

    static bool useCalculation;

    static void simpleSearch(QVector<ResultItem> &results, const QString &query, QList<ResultItem> &m_allItems);
    static void fuzzySearch(QVector<ResultItem> &results, const std::string &query, QList<ResultItem> &m_allItems);
    static void googleSearch(QVector<ResultItem> &results, const QString &query);
    static void tryCalculate(QVector<ResultItem> &results, const QString &query);
private:
    SearchFilters() = delete;
    ~SearchFilters() = delete;
};
