#include "structs/ResultItem.h"
#include "structs/ShortResultItem.h"

ResultItem ShortResultItem::toResultItem() const {
    ResultItem r;
    r.name = name;
    r.exec = exec;
    r.terminal = terminal;
    r.itemExecFrequency = itemExecFrequency;
    return r;
}
