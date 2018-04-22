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
    listeditorwidget.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    core/inmemorybackendconnection.cpp \
    core/websocket.cpp \
    core/dialogjsonreader.cpp \
    core/dialogjsonwriter.cpp \
    core/dialog.cpp \
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
    core/abstractdialognode.cpp \
    core/clientreplicanode.cpp \
    core/expectedwordsnode.cpp \
	core/phasenode.cpp \
	dialogeditor/dialoglisteditorwidget.cpp \
	usereditor/userlisteditorwidget.cpp

HEADERS  += \
    authwindow.h \
    listeditorwidget.h \
    logindialog.h \
    logger.h \
    mainwindow.h \
    core/dialog.h \
    core/ibackendconnection.h \
    core/inmemorybackendconnection.h \
    core/user.h \
    core/websocket.h \
    core/dialogjsonreader.h \
    core/dialogjsonwriter.h \
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
	core/abstractdialognode.h \
    core/clientreplicanode.h \
    core/expectedwordsnode.h \
    core/phasenode.h \
    listdatamodel.h \
	dialogeditor/dialoglisteditorwidget.h \
	usereditor/userlisteditorwidget.h

FORMS    += \
    authwindow.ui \
    listeditorwidget.ui \
    logindialog.ui \
    mainwindow.ui \
    dialogeditor/dialogeditorwindow.ui \
    usereditor/usereditordialog.ui \
    dialogeditor/expectedwordseditorwindow.ui \
    dialogeditor/expectedwordeditor.ui \
    dialogeditor/clientreplicaeditor.ui \
    dialogeditor/phaseeditorwindow.ui
