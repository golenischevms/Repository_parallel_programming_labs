TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

# Указываем компилятор MPI
QMAKE_CC = mpicc
QMAKE_CXX = mpic++

# Путь к заголовочным файлам MPI
INCLUDEPATH += /usr/include/openmpi

# Библиотеки MPI
LIBS += -lmpi_cxx -lmpi -lpthread -lrt

# Дополнительные флаги компиляции
QMAKE_CXXFLAGS += -Bsymbolic-functions

# Основной файл проекта
SOURCES += \
    main.cpp \
    task13.cpp \
    task14.cpp \
    task15.cpp \
    task16.cpp
