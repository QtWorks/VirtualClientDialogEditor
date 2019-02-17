#-------------------------------------------------
#
# Project created by QtCreator 2017-06-23T16:39:56
#
#-------------------------------------------------

VERSION = 1.0.0.1
RC_ICONS += ./icons/app_icon.ico

QT += core gui widgets network websockets sql

TARGET = DialogEditor
TEMPLATE = app

CONFIG(release, debug|release) {
	DESTDIR = release/bin

	QMAKE_PATH = $$QMAKE_QMAKE
	QMAKE_POST_LINK += $$dirname(QMAKE_PATH)/windeployqt $$OUT_PWD/$$DESTDIR/DialogEditor.exe
}

SOURCES += \
    listeditorwidget.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
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
	usereditor/userlisteditorwidget.cpp \
    core/backendconnection.cpp \
    waitingspinnerwidget.cpp \
	dialogeditor/graphlayout.cpp \
	settingsdialog.cpp \
	applicationsettings.cpp \
	clienteditor/clientlisteditorwidget.cpp \
    clienteditor/clienteditordialog.cpp \
	dialogeditor/dialogstabwidget.cpp \
	dialogeditor/dialoggraphicsinfostorage.cpp \
    usereditor/userstabwidget.cpp \
    dialogeditor/saveasdialog.cpp \
    clienteditor/groupslisteditorwidget.cpp \
    clienteditor/groupstabwidget.cpp \
    clienteditor/groupeditordialog.cpp \
    dialogeditor/groupsdialog.cpp \
    groupslistwidget.cpp

HEADERS  += \
    listeditorwidget.h \
    logindialog.h \
    logger.h \
    mainwindow.h \
    core/dialog.h \
    core/ibackendconnection.h \
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
	dialogeditor/dialoglisteditorwidget.h \
	usereditor/userlisteditorwidget.h \
    core/backendconnection.h \
    waitingspinnerwidget.h \
	dialogeditor/graphlayout.h \
	settingsdialog.h \
	applicationsettings.h \
    core/errorreplica.h \
    core/hashcombine.h \
	clienteditor/clientlisteditorwidget.h \
	core/client.h \
    clienteditor/clienteditordialog.h \
    dialogeditor/dialogstabwidget.h \
    dialogeditor/dialoggraphicsinfo.h \
	dialogeditor/dialoggraphicsinfostorage.h \
    usereditor/userstabwidget.h \
    dialogeditor/saveasdialog.h \
    clienteditor/groupslisteditorwidget.h \
    clienteditor/groupstabwidget.h \
    clienteditor/groupeditordialog.h \
    dialogeditor/groupsdialog.h \
    groupslistwidget.h \
    optional.h

FORMS    += \
    listeditorwidget.ui \
    logindialog.ui \
    mainwindow.ui \
    dialogeditor/dialogeditorwindow.ui \
    usereditor/usereditordialog.ui \
    dialogeditor/expectedwordseditorwindow.ui \
    dialogeditor/expectedwordeditor.ui \
    dialogeditor/clientreplicaeditor.ui \
    dialogeditor/phaseeditorwindow.ui \
	settingsdialog.ui \
    clienteditor/clienteditordialog.ui \
    dialogeditor/dialogstabwidget.ui \
    usereditor/userstabwidget.ui \
    dialogeditor/saveasdialog.ui \
    clienteditor/groupstabwidget.ui \
    clienteditor/groupeditordialog.ui \
    dialogeditor/groupsdialog.ui

RESOURCES += \
	resources.qrc
