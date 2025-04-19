#include "execbox.h"
#include <QDebug>
#include <QListWidgetItem>

ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent), m_ui(ui) {

    ReadFile();

    for(int i = 0;i<checkBoxes.size(); ++i){
        QCheckBox* checkbox = checkBoxes[i];
        connect(checkbox, &QCheckBox::toggled, this, [=](bool checked) {
            if (checked) {
                CheckBoxProcessMap[checkbox] = new QProcess(this);
                StartSession(CheckBoxProcessMap[checkbox], commands[i], checkbox->text());
            }
            else {
                StopSession(CheckBoxProcessMap[checkbox], checkbox->text());
                CheckBoxProcessMap[checkbox] = nullptr;
            }
        });
    }
}
//this executes the command without a terminal window.
void ExecBox::StartSession(QProcess* process, const QString& cmd, const QString& label){
    qDebug()<<cmd;

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
        m_ui->TerminalDisplay->appendPlainText(output);
    });

    connect(process, &QProcess::readyReadStandardError, this, [=]() {
        QString errorOutput = process->readAllStandardError();
        m_ui->TerminalDisplay->appendPlainText("[ERROR] " + errorOutput);
    });


    // Add label to running list
    m_ui->runningScriptsList->addItem(label);

}
void ExecBox::StopSession(QProcess* process, const QString& label){

    process->terminate();
    if (!process->waitForFinished(3000)) {
        process->kill();
        process->waitForFinished(3000);
    }
    delete process;

    // Remove from the running scripts list
    for (int i = 0; i < m_ui->runningScriptsList->count(); ++i) {
        if (m_ui->runningScriptsList->item(i)->text() == label) {
            delete m_ui->runningScriptsList->takeItem(i);
            break;
        }
    }

}

void ExecBox::ReadFile(){

    QFile File("config.json");
    QByteArray Bytes;
    if(File.open(QIODevice::ReadOnly | QIODevice::Text)){
        Bytes = File.readAll();
        File.close();

        QJsonParseError JsonError;
        QJsonDocument Document =  QJsonDocument::fromJson(Bytes, &JsonError);
        if(JsonError.error != QJsonParseError::NoError){
            qDebug()<<"Error in Json Data: "<<JsonError.errorString();
            return;
        }
        else{
            qDebug()<<"No error found in Json Data";
        }
        if (!Document.isArray()) {
            qDebug() << "Expected a JSON array";
            return;
        }
        QJsonArray array = Document.array();

        QCheckBox* checkbox = nullptr;
        for(int i = 0; i < array.size(); i++){
            QJsonObject obj = array.at(i).toObject();
            QString label = obj["label"].toString();
            QString command = obj["command"].toString();
            qDebug() << "Label:" << label;
            qDebug() << "Command:" << command;
            checkbox = new QCheckBox(this);
            checkbox->setText(label);
            checkBoxes.append(checkbox);
            commands.append(command);
            SetupUI(checkbox);
            CheckBoxProcessMap[checkbox] = nullptr;
            checkbox = nullptr;
        }
    }
}

void ExecBox::SetupUI(QCheckBox* checkbox){
    // checkbox->setStyleSheet("color: black;");
    checkbox->setStyleSheet(
        "QCheckBox { color: black; }"
        "QCheckBox:hover { color: blue; font-weight: bold; }"
        );
    m_ui->buttons->addWidget(checkbox);

    // Connect item click to show output
    connect(m_ui->runningScriptsList, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
        QString scriptName = item->text();
        //placeholder
        m_ui->outputDisplay->clear();
        m_ui->outputDisplay->appendPlainText("Showing output for: " + scriptName);
    });

    update();
}
