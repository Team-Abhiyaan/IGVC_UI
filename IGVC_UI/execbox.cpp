#include "execbox.h"
#include "spoiler.h"

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


    createSpoiler(label);

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
        QLayoutItem* item = m_ui->runningScriptsList->itemAt(i);
        if(item){
            QWidget* widget = item->widget();
            if (Spoiler* spoiler = qobject_cast<Spoiler*>(widget)) {
                if(spoiler->toggleButton.text()==label){
                    m_ui->runningScriptsList->takeAt(i);
                    delete widget;
                    delete item;
                    break;
                }
            }
        }
    }
    ScriptOutputMap.remove(label);
    update();
    // qobject_cast<Spoiler*>(m_ui->runningScriptsList->widget())

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


    update();
}
void ExecBox::createSpoiler(const QString label){
    Spoiler *spoiler = new Spoiler(label);

    // Add content to the spoiler
    QVBoxLayout *spoilerContent = new QVBoxLayout();
    QSlider *slider = new QSlider(Qt::Horizontal, this);

    // Set the range of the slider
    slider->setMinimum(0);   // minimum value
    slider->setMaximum(100); // maximum value
    slider->setValue(50);    // initial value
    slider->setTickPosition(QSlider::TicksBelow); // show ticks below the slider
    slider->setTickInterval(10);  // interval between ticks
    slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);


    // Create a label to display the slider value
    QLabel *param_name = new QLabel("Value: 50", this);

    // Connect the slider's valueChanged signal to update the label
    connect(slider, &QSlider::valueChanged, [param_name](int value) {
        param_name->setText("Value: " + QString::number(value));
    });
    param_name->setStyleSheet("QLabel {"
                         "font-size: 16px;"
                         "color: #000000;"
                         "}");
    spoilerContent->addWidget(param_name);

    spoilerContent->addWidget(slider);

    spoiler->setContentLayout(*spoilerContent);
    connect(&spoiler->toggleButton, &QToolButton::clicked, this, [=](bool checked) {
        currentSelectedLabel = label;
        lastShownOutput.clear();
        qDebug() << "Spoiler clicked:" << label;
    });

    // Add spoiler to layout
    m_ui->runningScriptsList->addWidget(spoiler);
}
