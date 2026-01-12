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
const QMap<QString, std::function<void(ResultItem&, const QString&)>> ResultItem::entryFieldHandlers = {
    {"Name", [](ResultItem &e, const QString &v){ e.name = v; }},
    {"GenericName", [](ResultItem &e, const QString &v){ e.genericName = v; }},
    {"Comment", [](ResultItem &e, const QString &v){ e.comment = v; }},
    {"Exec", [](ResultItem &e, const QString &v){ e.exec = v; }},
    {"Type", [](ResultItem &e, const QString &v){ e.type = v; }},
    {"Categories", [](ResultItem &e, const QString &v){ e.categories = v; }},
    {"Terminal", [](ResultItem &e, const QString &v){ e.terminal = (v.toLower() == "true"); }},
    {"NoDisplay", [](ResultItem &e, const QString &v){ e.noDisplay = (v.toLower() == "true"); }},
    {"MimeType", [](ResultItem &e, const QString &v){ e.mimeType = v; }},
    {"StartupNotify", [](ResultItem &e, const QString &v){ e.startupNotify = v; }},
    {"Hidden", [](ResultItem &e, const QString &v){ e.hidden = v; }},
    {"Keywords", [](ResultItem &e, const QString &v){ e.keywords = v; }},
    {"Path", [](ResultItem &e, const QString &v){ e.path = v; }},
    {"Icon", [](ResultItem &e, const QString &v){
        e.icon = KIconLoader::global()->iconPath(v, KIconLoader::Desktop);
        if (e.icon.isEmpty()) {
            e.icon = KIconLoader::global()->iconPath(v, KIconLoader::Any);
            if (e.icon.isEmpty())
                e.icon = KIconLoader::global()->iconPath("preferences-desktop-display", KIconLoader::Any);
        }
    }}
};
