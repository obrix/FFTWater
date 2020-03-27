#include "waterfftvisu.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  waterFFTvisu w(NULL);
  w.show();
  return a.exec();
}
