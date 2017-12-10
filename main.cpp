#include "mainwindow.h"
#include "core/inmemorybackendconnection.h"
#include <QApplication>
#include <memory>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

	IBackendConnectionSharedPtr backendConection = std::make_shared<InMemoryBackendConnection>();

	MainWindow window(backendConection);
	window.show();

    return a.exec();
}
