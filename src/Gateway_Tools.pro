#-------------------------------------------------
#
# Project created by QtCreator 2020-08-04T13:59:05
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gateway_Tools
TEMPLATE = app


SOURCES += main.cpp\
        clientwidget.cpp

HEADERS  += clientwidget.h

FORMS    += clientwidget.ui

CONFIG   += C++11

#RC_ICONS = favicon.ico
RC_ICONS = mms.ico
