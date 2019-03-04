#include "mainwindow.h"
#include "core/backendconnection.h"
#include "dialogeditor/dialoggraphicsinfostorage.h"
#include "applicationsettings.h"
#include <QApplication>
#include <memory>
#include <iostream>

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	switch (type)
	{
	case QtCriticalMsg:
		std::cerr << QString("Critical: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function).toStdString();
		break;
	case QtFatalMsg:
		std::cerr << QString("Fatal: %1 (%2:%3, %4)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function).toStdString();
		abort();
	}
}

int main(int argc, char* argv[])
{
	qInstallMessageHandler(messageHandler);
	QApplication app(argc, argv);

	QTranslator translator;
	if (translator.load("qt_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
		app.installTranslator(&translator);
	}

	ApplicationSettings settings;

	IBackendConnectionSharedPtr backendConection = std::make_shared<Core::BackendConnection>(QUrl(settings.hostname()));

	const QString dbPath = app.applicationDirPath() + "\\" + "graphics.sqlite";
	auto dialogGraphicsInfoStorage = std::make_shared<DialogGraphicsInfoStorage>(dbPath);
	dialogGraphicsInfoStorage->open();

	MainWindow window(&settings, backendConection, dialogGraphicsInfoStorage);
	window.show();

	return app.exec();
}
