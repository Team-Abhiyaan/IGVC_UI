#include "execbox.h"
#include <QDebug>

ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent), m_ui(ui) {


    CheckBoxCommandMap[m_ui->elecstack] = "ros2 launch elecstack elec_launch.py";
    CheckBoxCommandMap[m_ui->robot_desc] = "ros2 launch robot_desc robot_launch.py";
    CheckBoxCommandMap[m_ui->zed] = "sudo -E env bash -c 'source /opt/ros/humble/setup.bash; ros2 launch zed_wrapper zed_camera.launch.py camera_model:=zed2i publish_tf:=false publish_map_tf:=false'";
    CheckBoxCommandMap[m_ui->lidar] = "ros2 launch sllidar_ros2 sllidar_a2m12_launch.py";
    CheckBoxCommandMap[m_ui->lidar_range_filter] = "ros2 launch laser_filters range_filter_example.launch.py";
    CheckBoxCommandMap[m_ui->lidar_angular_filter] = "ros2 launch laser_filters angular_filter_example.launch.py";
    CheckBoxCommandMap[m_ui->lane_detection] = "ros2 launch lane_detection start.py";
    CheckBoxCommandMap[m_ui->pothole_detection] = "ros2 run lane_pothole_detection potholes";
    CheckBoxCommandMap[m_ui->nav] = "ros2 launch sensor_fusion fusion.launch.py";
    CheckBoxCommandMap[m_ui->pathfinder] = "ros2 run path_finder find_path";
    CheckBoxCommandMap[m_ui->gps] = "ros2 run demo_nodes_cpp talker";
    CheckBoxCommandMap[m_ui->go] = "ros2 run nav_commander go";

    CheckBoxProcessMap[m_ui->elecstack] = nullptr;
    CheckBoxProcessMap[m_ui->robot_desc] = nullptr;
    CheckBoxProcessMap[m_ui->zed] = nullptr;
    CheckBoxProcessMap[m_ui->lidar] = nullptr;
    CheckBoxProcessMap[m_ui->lidar_range_filter] = nullptr;
    CheckBoxProcessMap[m_ui->lidar_angular_filter] = nullptr;
    CheckBoxProcessMap[m_ui->lane_detection] = nullptr;
    CheckBoxProcessMap[m_ui->pothole_detection] = nullptr;
    CheckBoxProcessMap[m_ui->nav] = nullptr;
    CheckBoxProcessMap[m_ui->pathfinder] = nullptr;
    CheckBoxProcessMap[m_ui->gps] = nullptr;
    CheckBoxProcessMap[m_ui->go] = nullptr;


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


    QString fullCommand;

    if (cmd.contains("source") || cmd.contains("sudo"))
    {   fullCommand = cmd;}
    else
    {   fullCommand = "source /opt/ros/jazzy/setup.bash && " + cmd;}

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
