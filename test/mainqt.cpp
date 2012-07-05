#include <QApplication>

 #include "mainwindow.h"
#include <string.h>
#include <iostream>
using namespace std;
using namespace rtps;
 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
	string filepath=argv[0];
	unsigned int pos=0;
#ifdef WIN32
	pos=filepath.rfind("\\");
#else
	pos=filepath.rfind("/");
#endif
	filepath=filepath.substr(0,pos);
    cout<<"argv = "<<argv[0]<<endl;
    cout<<"filepath = "<<filepath<<endl;
     MainWindow mainWin(filepath);
     mainWin.show();
     return app.exec();
 }
