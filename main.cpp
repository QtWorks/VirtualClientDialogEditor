#include "mainwindow.h"
#include "core/backendconnection.h"
#include <QApplication>
#include <memory>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

	IBackendConnectionSharedPtr backendConection = std::make_shared<Core::BackendConnection>(QUrl("ws://vcappdemo.herokuapp.com/"));

	MainWindow window(backendConection);
	window.show();

    return a.exec();
}
