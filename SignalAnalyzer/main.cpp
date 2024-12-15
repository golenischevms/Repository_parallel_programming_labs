#include "mainwindow.h"
#include <mpi.h>

#include <QApplication>
#include <QLocale>
#include <QTranslator>


int main(int argc, char *argv[]) {
    // Инициализация MPI
    MPI_Init(&argc, &argv);

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SignalAnalyzer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    int result = a.exec();

    // Завершаем работу MPI
    MPI_Finalize();

    return result;
}

