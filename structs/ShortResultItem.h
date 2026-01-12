#pragma once
#include <QString>

struct ResultItem;

struct ShortResultItem{
    QString name;
    QString exec;
    bool terminal = false;
    int itemExecFrequency;

    ResultItem toResultItem() const;
};
