#include "mainwindow.h"
#include "core/backendconnection.h"
#include "dialogeditor/dialoggraphicsinfostorage.h"
#include "applicationsettings.h"
#include <QApplication>
#include <memory>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	ApplicationSettings settings;

	IBackendConnectionSharedPtr backendConection = std::make_shared<Core::BackendConnection>(QUrl(settings.hostname()));

	const QString dbPath = a.applicationDirPath() + "\\" + "graphics.sqlite";
	auto dialogGraphicsInfoStorage = std::make_shared<DialogGraphicsInfoStorage>(dbPath);
	dialogGraphicsInfoStorage->open();

	MainWindow window(&settings, backendConection, dialogGraphicsInfoStorage);
	window.show();

	return a.exec();
}
