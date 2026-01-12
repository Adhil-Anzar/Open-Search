#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QIcon>

#include "models/ResultsModel.h"

int main(int argc, char *argv[])
{
    using namespace std::chrono;

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/open-search.svg"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    ResultsModel resultsModel;
    qmlRegisterType<ResultsModel>("Results", 1, 0, "ResultsModel");
    resultsModel.getResults();
    engine.rootContext()->setContextProperty("resultsModel", &resultsModel);


    engine.loadFromModule("OpenSearch", "Main");

    return app.exec();
}
