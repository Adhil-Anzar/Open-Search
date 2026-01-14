#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QIcon>
#include <chrono>

#include "models/ResultsModel.h"
#include "models/ItemUtils.h"
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

    auto start = std::chrono::high_resolution_clock::now();
    ResultsModel resultsModel;
    resultsModel.getResults();
    qmlRegisterType<ResultsModel>("Results", 1, 0, "ResultsModel");

    ItemUtils itemUtils;
    engine.rootContext()->setContextProperty("ItemUtils", &itemUtils);

    engine.rootContext()->setContextProperty("resultsModel", &resultsModel);


    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    qDebug() << "Took " << ms << " ms\n";

    engine.loadFromModule("OpenSearch", "Main");

    return app.exec();
}
