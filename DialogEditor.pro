#-------------------------------------------------
#
# Project created by QtCreator 2017-06-23T16:39:56
#
#-------------------------------------------------

QT += core gui widgets network websockets

TARGET = DialogEditor
TEMPLATE = app

#QMAKE_CXXFLAGS += "-include logger.h"

Release:QMAKE_POST_LINK += bin/windeployqt $$OUT_PWD/$$DESTDIR

SOURCES += \
    authwindow.cpp \
    editablelistitem.cpp \
    listeditorwidget.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    core/dialogmodel.cpp \
    core/inmemorybackendconnection.cpp \
    core/user.cpp \
    core/websocket.cpp \
    dialogeditor/arrowlinegraphicsitem.cpp \
    dialogeditor/clientreplicanodegraphicsitem.cpp \
    dialogeditor/dialogeditorwindow.cpp \
    dialogeditor/dialoggraphicsscene.cpp \
    dialogeditor/expectedwordsnodegraphicsitem.cpp \
    dialogeditor/fontmetricsheighteliding.cpp \
    dialogeditor/nodegraphicsitem.cpp \
    usereditor/usereditordialog.cpp \
    dialogeditor/expectedwordseditorwindow.cpp \
    dialogeditor/expectedwordeditor.cpp \
    dialogeditor/clientreplicaeditor.cpp \
    dialogeditor/dialogconstructorgraphicsscene.cpp \
    dialogeditor/nodegraphicsitemmimedata.cpp \
    dialogeditor/arrowlinegraphicsitemmimedata.cpp \
	dialogeditor/zoomablegraphicsview.cpp \
	dialogeditor/phasegraphicsitem.cpp \
    dialogeditor/phaseeditorwindow.cpp \
    core/dialogjsonreader.cpp \
    core/dialogjsonwriter.cpp

HEADERS  += \
    authwindow.h \
    editablelistitem.h \
    listeditorwidget.h \
    logindialog.h \
    mainwindow.h \
    core/dialog.h \
    core/dialogmodel.h \
    core/ibackendconnection.h \
    core/idialogmodel.h \
    core/inmemorybackendconnection.h \
    core/user.h \
    core/websocket.h \
	logger.h \
    dialogeditor/arrowlinegraphicsitem.h \
    dialogeditor/clientreplicanodegraphicsitem.h \
    dialogeditor/dialogeditorwindow.h \
    dialogeditor/dialoggraphicsscene.h \
    dialogeditor/expectedwordsnodegraphicsitem.h \
    dialogeditor/fontmetricsheighteliding.h \
    dialogeditor/nodegraphicsitem.h \
    usereditor/usereditordialog.h \
    dialogeditor/expectedwordseditorwindow.h \
    dialogeditor/expectedwordeditor.h \
    dialogeditor/clientreplicaeditor.h \
    dialogeditor/dialogconstructorgraphicsscene.h \
    dialogeditor/nodegraphicsitemmimedata.h \
    dialogeditor/arrowlinegraphicsitemmimedata.h \
	dialogeditor/zoomablegraphicsview.h \
	dialogeditor/phasegraphicsitem.h \
    dialogeditor/phaseeditorwindow.h \
    core/dialogjsonreader.h \
    core/dialogjsonwriter.h

FORMS    += \
    authwindow.ui \
    editablelistitem.ui \
    listeditorwidget.ui \
    logindialog.ui \
    mainwindow.ui \
    dialogeditor/dialogeditorwindow.ui \
    usereditor/usereditordialog.ui \
    dialogeditor/expectedwordseditorwindow.ui \
    dialogeditor/expectedwordeditor.ui \
    dialogeditor/clientreplicaeditor.ui \
    dialogeditor/phaseeditorwindow.ui
