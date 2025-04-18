#include "execbox.h"
#include <QDebug>

ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent), m_ui(ui) {


    CheckBoxCommandMap[m_ui->electrical] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->zed] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->gps] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->waypoint_following] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->navigation] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->lane_detection] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->lane_following] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->idk_one_extra] = "ros2 run demo_nodes_cpp talker";

    CheckBoxProcessMap[m_ui->electrical] = nullptr;
    CheckBoxProcessMap[m_ui->zed] = nullptr;
    CheckBoxProcessMap[m_ui->gps] = nullptr;
    CheckBoxProcessMap[m_ui->waypoint_following] = nullptr;
    CheckBoxProcessMap[m_ui->navigation] = nullptr;
    CheckBoxProcessMap[m_ui->lane_detection] = nullptr;
    CheckBoxProcessMap[m_ui->lane_following] = nullptr;
    CheckBoxProcessMap[m_ui->idk_one_extra] = nullptr;

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

    connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = process->readAllStandardOutput();
        m_ui->outputDisplay->appendPlainText(output);
    });

    connect(process, &QProcess::readyReadStandardError, this, [=]() {
        QString errorOutput = process->readAllStandardError();
        m_ui->outputDisplay->appendPlainText("[ERROR] " + errorOutput);
    });

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
