#-------------------------------------------------
#
# Project created by QtCreator 2020-08-03T14:17:32
#
#-------------------------------------------------

QT       += core gui sql network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = blockchainRCP
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

QMAKE_CXXFLAGS += -Wno-missing-braces \
                  -Wall \
                  -Wunused \
                  -Wfloat-equal \
                  -Wwrite-strings


SOURCES += \
        coinview.cpp \
        configdialog.cpp \
        dialogbalance.cpp \
        dialogmasterkey.cpp \
        dialogpassword.cpp \
        dialogtokenbranch.cpp \
        hdwallet.cpp \
        main.cpp \
        mainwindow.cpp \
        printers.cpp \
        rpc.cpp \
        utility.cpp \
        walletdb.cpp

HEADERS += \
        coinview.h \
        configdialog.h \
        dialogbalance.h \
        dialogmasterkey.h \
        dialogpassword.h \
        dialogtokenbranch.h \
        hdwallet.h \
        json.hpp \
        mainwindow.h \
        printers.h \
        rpc.h \
        utility.h \
        walletdb.h

FORMS += \
        coinview.ui \
        configdialog.ui \
        dialogbalance.ui \
        dialogmasterkey.ui \
        dialogpassword.ui \
        dialogtokenbranch.ui \
        mainwindow.ui


unix:!macx: LIBS += -L$$PWD/Multy-Core/bin/multy_core/ -lmulty_core
INCLUDEPATH += $$PWD/Multy-Core
DEPENDPATH += $$PWD/Multy-Core
unix:!macx: PRE_TARGETDEPS += $$PWD/Multy-Core/bin/multy_core/libmulty_core.a

unix:!macx:LIBS += -L$$PWD/Multy-Core/bin/third-party/ -lmini-gmp
INCLUDEPATH += $$PWD/Multy-Core/third-party/mini-gmp
DEPENDPATH += $$PWD/Multy-Core/third-party/mini-gmp
unix:!macx: PRE_TARGETDEPS += $$PWD/Multy-Core/bin/third-party/libmini-gmp.a

unix:!macx: LIBS += -L$$PWD/Multy-Core/bin/third-party/ -lccan
INCLUDEPATH += $$PWD/Multy-Core/third-party/ccan
DEPENDPATH += $$PWD/Multy-Core/third-party/ccan
unix:!macx: PRE_TARGETDEPS += $$PWD/Multy-Core/bin/third-party/libccan.a

unix:!macx: LIBS += -L$$PWD/Multy-Core/bin/third-party/ -lkeccak-tiny
INCLUDEPATH += $$PWD/Multy-Core/third-party/keccak-tiny
DEPENDPATH += $$PWD/Multy-Core/third-party/keccak-tiny
unix:!macx: PRE_TARGETDEPS += $$PWD/Multy-Core/bin/third-party/libkeccak-tiny.a

unix:!macx: LIBS += -L$$PWD/Multy-Core/bin/third-party/libwally-core/ -lsecp256k1
INCLUDEPATH += $$PWD/Multy-Core/third-party/libwally-core/include
DEPENDPATH += $$PWD/Multy-Core/third-party/libwally-core/include
unix:!macx: PRE_TARGETDEPS += $$PWD/Multy-Core/bin/third-party/libwally-core/libsecp256k1.a

unix:!macx: LIBS += -L$$PWD/Multy-Core/bin/third-party/libwally-core/ -llibwally-core
INCLUDEPATH += $$PWD/Multy-Core/third-party/libwally-core/include
DEPENDPATH += $$PWD/Multy-Core/third-party/libwally-core/include
unix:!macx: PRE_TARGETDEPS += $$PWD/Multy-Core/bin/third-party/libwally-core/liblibwally-core.a


unix:!macx: LIBS += -lcurl
