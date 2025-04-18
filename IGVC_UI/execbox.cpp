#include "execbox.h"


ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent) {


    CheckBoxCommandMap[ui->electrical] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->zed] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->gps] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->waypoint_following] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->navigation] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->lane_detection] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->lane_following] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[ui->idk_one_extra] = "ros2 run demo_nodes_cpp talker";

    CheckBoxProcessMap[ui->electrical] = nullptr;
    CheckBoxProcessMap[ui->zed] = nullptr;
    CheckBoxProcessMap[ui->gps] = nullptr;
    CheckBoxProcessMap[ui->waypoint_following] = nullptr;
    CheckBoxProcessMap[ui->navigation] = nullptr;
    CheckBoxProcessMap[ui->lane_detection] = nullptr;
    CheckBoxProcessMap[ui->lane_following] = nullptr;
    CheckBoxProcessMap[ui->idk_one_extra] = nullptr;

    for (auto i = CheckBoxProcessMap.begin(), end = CheckBoxProcessMap.end(); i != end; ++i){
        QCheckBox* checkbox = i.key();
        connect(checkbox, &QCheckBox::toggled, this, [=](bool checked) {
            if (checked) {
                CheckBoxProcessMap[checkbox] = new QProcess(this);
                StartSession(CheckBoxProcessMap[checkbox], CheckBoxCommandMap[checkbox]);
            }
            else {
                StopSession(CheckBoxProcessMap[checkbox]);
                CheckBoxProcessMap[checkbox] = nullptr;
            }
        });
    }
}
//this executes the command without a terminal window, can be checked with listener node as talker node is being used here.
void ExecBox::StartSession(QProcess* process, const QString& cmd){
    qDebug()<< "checked";
    QString fullCommand = "source /opt/ros/jazzy/setup.bash && " + cmd;
    process->setWorkingDirectory("/home/");
    process->start("bash", QStringList() << "-c" << fullCommand);

    if (!process->waitForStarted()) {
        qDebug() << "Failed to execute "<<cmd;
    }
}
void ExecBox::StopSession(QProcess* process){
    qDebug()<< "unchecked";
    process->terminate();
    if (!process->waitForFinished(3000)) {
        process->kill();
        process->waitForFinished(3000);
    }
    delete process;
    qDebug()<<"Process terminated";
}

// there is one column on the right side of UI called "commands", where the corresponding commands has to be displayed, Logic is not implementd as of now.
