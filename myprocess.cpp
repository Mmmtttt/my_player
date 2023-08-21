#include "player.h"
#include <QCoreApplication>



int main(int argc, char *argv[])
{
    QApplication  a(argc, argv);



    // 获取传递的参数
    QStringList arguments = a.arguments();
    if (arguments.size() < 2)
    {
        qDebug() << "No argument provided.";
        return 1;
    }

    QString argument = arguments.at(1); // 第一个参数是程序名，第二个参数是传递的字符串

    Player player(NULL,argument.toStdString());
    player.show();

    emit player.actionSignal();

    // 在这里可以进行你希望在单独进程中执行的操作
    // ...
    //QObject::connect(&player, &Player::exitSignal, &a, &exit); // 连接退出信号

    return a.exec();
}
