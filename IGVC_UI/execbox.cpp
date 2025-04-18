#include "execbox.h"


ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent) {

    QProcess elecrical_process;
    connect(ui->electrical, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(electrical_process, "cmd");
        }
        else {
            StopSession(electrical_process);
        }
        });
}
//this executes the command without a terminal window, can be checked with listener node as talker node is being used here.
void ExecBox::StartSession(QProcess& process, const QString cmd){
    qDebug()<< "checked";
    process.setWorkingDirectory("/home/");
    process.start("bash", QStringList() << "-c"
                    << "source /opt/ros/jazzy/setup.bash && ros2 run demo_nodes_cpp talker");

    if (!process.waitForStarted()) {
        qDebug() << "Failed to start talker node!";
    }
}
void ExecBox::StopSession(QProcess& process){
    qDebug()<< "unchecked";
    process.terminate();
    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished(3000);
    }
    qDebug()<<"Process terminated";
}

// there is one column on the right side of UI called "commands", where the corresponding commands has to be displayed, Logic is not implementd as of now.
