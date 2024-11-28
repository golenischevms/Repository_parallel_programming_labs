TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS += -fopenmp
QMAKE_CXXFLAGS += -fopenmp

SOURCES += \
        main.cpp \
        task2.cpp \
        task3.cpp
