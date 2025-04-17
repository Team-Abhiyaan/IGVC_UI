#include "execbox.h"


ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent) {

    connect(ui->electrical, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(electrical_process, "cmd");
        }
        else {
            StopSession(electrical_process);
        }
        });
    connect(ui->zed, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(zed_process, "cmd");
        }
        else {
            StopSession(zed_process);
        }
    });
    connect(ui->waypoint_following, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(waypoint_following_process,"cmd");
        }
        else {
            StopSession(waypoint_following_process);
        }
    });
    connect(ui->gps, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(gps_process,"cmd");
        }
        else {
            StopSession(gps_process);
        }
    });
    connect(ui->navigation, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(navigation_process, "cmd");
        }
        else {
            StopSession(navigation_process);
        }
    });
    connect(ui->lane_detection, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(lane_detection_process, "cmd");
        }
        else {
            StopSession(lane_detection_process);
        }
    });
    connect(ui->lane_following, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            StartSession(lane_following_process, "cmd");
        }
        else {
            StopSession(lane_following_process);
        }
    });

}
//this executes the command without a terminal window, can be checked with listener node as talker node is being used here.
void ExecBox::StartSession(QProcess* process, const QString cmd){
    process = new QProcess(this);
    process->setWorkingDirectory("/home/");
    process->start("bash", QStringList() << "-c"
                    << "source /opt/ros/jazzy/setup.bash && ros2 run demo_nodes_cpp talker");

    if (!process->waitForStarted()) {
        qDebug() << "Failed to start talker node!";
    }
    qDebug()<< "checked";
}
// this is not working as of now, Killing is not working.
void ExecBox::StopSession(QProcess* process){
    if (process) {
        process->kill();
        process->waitForFinished(3000);
        delete process;
        process = nullptr;
    }
    qDebug()<< "unchecked";
}

// there is one column on the right side of UI called "commands", where the corresponding commands has to be displayed, Logic is not implementd as of now.
