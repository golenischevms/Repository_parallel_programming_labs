TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

# Флаги компилятора и линковки для OpenMP
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += \
        main.cpp \
        task6.cpp \
        task7.cpp \
        task8.cpp