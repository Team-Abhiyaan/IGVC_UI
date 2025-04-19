#include "execbox.h"

ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent), m_ui(ui) {

    //reading json file
    ReadFile();

    //connecting each checkbox to their processes.
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

    updateTimer= new QTimer(this);
    updateTimer->setInterval(500);

    connect(updateTimer, &QTimer::timeout, this, [=]() {
        if (!currentSelectedLabel.isEmpty()) {
            QString fullOutput = ScriptOutputMap.value(currentSelectedLabel);
            if (fullOutput != lastShownOutput) {
                m_ui->TerminalDisplay->clear();
                m_ui->TerminalDisplay->appendPlainText(fullOutput);
                lastShownOutput = fullOutput;
            }
        }
    });

    updateTimer->start();
}
// this function executes the command without a terminal window.
void ExecBox::StartSession(QProcess* process, const QString& cmd, const QString& label){

    QString fullCommand;

    // Executes both sourcing ros2 and given command execution
    if (cmd.contains("source") || cmd.contains("sudo"))
    {   fullCommand = cmd;}
    else
    {   fullCommand = "source /opt/ros/jazzy/setup.bash && " + cmd;}

    process->setWorkingDirectory(working_directory);

    process->start("bash", QStringList() << "-c" << fullCommand);

    if (!process->waitForStarted()) {
        qDebug() << "Failed to execute "<<cmd;
    }

    connect(process, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = process->readAllStandardOutput();
        ScriptOutputMap[label] += output;
    });

    connect(process, &QProcess::readyReadStandardError, this, [=]() {
        QString errorOutput = "[ERROR] " + process->readAllStandardError();
        ScriptOutputMap[label] += errorOutput;
    });



    // Add label to running list
    m_ui->runningScriptsList->addItem(label);

}
void ExecBox::StopSession(QProcess* process, const QString& label){

    //terminate the given process
    process->terminate();

    //if not terminated within 3s, kill the process
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

    ScriptOutputMap.remove(label);

}

// this function reads the json file
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
        if (!Document.isObject()) {
            qDebug() << "Expected a JSON object at root";
            return;
        }

        QJsonObject rootObj = Document.object();

        working_directory = rootObj["working_directory"].toString();
        qDebug()<<"Current working directory:"<<working_directory;

        if(!rootObj["commands"].isArray()){
            qDebug()<<"Expected 'commands' to be an array";
            return;
        }

        QJsonArray array = rootObj["commands"].toArray();

        QCheckBox* checkbox = nullptr;
        for(int i = 0; i < array.size(); i++){
            QJsonObject obj = array.at(i).toObject();
            QString label = obj["label"].toString();
            QString command = obj["command"].toString();
            checkbox = new QCheckBox(this);
            checkbox->setText(label);

            // after getting each checkbox and command,
            // UI is updated and command and checkbox are added to their corresponding list
            checkBoxes.append(checkbox);
            commands.append(command);

            SetupUI(checkbox);
            CheckBoxProcessMap[checkbox] = nullptr;
            checkbox = nullptr;
        }
    }
}

void ExecBox::SetupUI(QCheckBox* checkbox){

    checkbox->setStyleSheet(
        "QCheckBox { color: black; }"
        "QCheckBox:hover { color: blue; font-weight: bold; }"
        );
    m_ui->buttons->addWidget(checkbox);

    // Connect item click to show output
    connect(m_ui->runningScriptsList, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
        currentSelectedLabel = item->text();
        lastShownOutput.clear();
    });

    update();
}
