TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

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

SOURCES += \
        main.cpp \
        task31.cpp \
        task32.cpp
