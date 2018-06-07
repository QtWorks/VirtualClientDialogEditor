#include "mainwindow.h"
#include "core/backendconnection.h"
#include "applicationsettings.h"
#include <QApplication>
#include <memory>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	ApplicationSettings settings;

	IBackendConnectionSharedPtr backendConection = std::make_shared<Core::BackendConnection>(QUrl(settings.hostname()));

	MainWindow window(&settings, backendConection);
	window.show();

	return a.exec();
}
