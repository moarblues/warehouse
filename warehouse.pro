#-------------------------------------------------
#
# Project created by QtCreator 2013-07-13T10:51:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = warehouse
TEMPLATE = app

DESTDIR = bin

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    client.cpp \
    dlg_settings.cpp \
    files.cpp \
    dlg_selectrecord.cpp \
    dlg_alterproductdata.cpp \
    dlg_addgoodsmanual.cpp \
    dlg_addproduct.cpp

HEADERS  += \
    mainwindow.h \
    client.h \
    dlg_settings.h \
    files.h \
    dlg_selectrecord.h \
    dlg_alterproductdata.h \
    dlg_addgoodsmanual.h \
    dlg_addproduct.h

FORMS    += mainwindow.ui \
    dlg_settings.ui \
    dlg_selectrecord.ui \
    dlg_alterproductdata.ui \
    dlg_addgoodsmanual.ui \
    dlg_addproduct.ui

unix: LIBS += -Llib/freexl/ -lfreexl

INCLUDEPATH += lib/incl/freexl
DEPENDPATH += lib/incl/freexl

unix: PRE_TARGETDEPS += lib/freexl/libfreexl.a

include(qtxlsx/qtxlsx.pri)
