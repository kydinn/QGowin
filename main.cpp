#include "fpga_update_page.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	FpgaUpdatePage w;
	w.show();
    return a.exec();
}
