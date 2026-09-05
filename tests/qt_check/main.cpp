#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    std::cout << "KaLara Arch Step 00: Qt6 Core Environment Check OK: " << QT_VERSION_STR << std::endl;
    return 0;
}
