QT       += core gui printsupport opengl

DEFINES += QCUSTOMPLOT_USE_OPENGL

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Флаги компилятора и линковки для OpenMP
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

# Указываем компилятор MPI
QMAKE_CC = mpicc
QMAKE_CXX = mpic++

# Путь к заголовочным файлам MPI
INCLUDEPATH += /usr/include/openmpi

# Библиотеки MPI
LIBS += -lmpi_cxx -lmpi -lpthread -lrt

# Дополнительные флаги компиляции
QMAKE_CXXFLAGS += -Bsymbolic-functions

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    mainwindow.h \
    qcustomplot.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    SignalAnalyzer_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
