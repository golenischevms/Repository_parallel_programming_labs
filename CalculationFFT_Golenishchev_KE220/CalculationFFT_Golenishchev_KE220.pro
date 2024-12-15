TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

# Флаги компилятора и линковки для OpenMP
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

# Пути к заголовкам и библиотекам OpenACC
INCLUDEPATH += /snap/freecad/1202/usr/lib/gcc/x86_64-linux-gnu/11/include
LIBS += -L/snap/freecad/1202/usr/lib/gcc/x86_64-linux-gnu/11/lib64

# Пути к заголовкам и библиотекам FFTW
INCLUDEPATH += /path/to/fftw/include
LIBS += -L/path/to/fftw/lib -lfftw3 -lm

# Источник кода
SOURCES += \
        main.cpp
