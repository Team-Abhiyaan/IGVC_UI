#include "execbox.h"
#include "spoiler.h"

ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent), m_ui(ui) {

    //reading json file
    ReadJSON();
    ReadYAML();
    // commandParameterMap["elecstack"].append(parameter("test" + QString("elecstack"), 5, 10, 7));
    // commandParameterMap["robot_desc"].append(parameter("test" + QString("robot_desc"), 0, 5, 3));
    // commandParameterMap["zed"].append(parameter("test" + QString("zed"), 5, 15, 10));
    // commandParameterMap["lidar"].append(parameter("test" + QString("lidar"), 10, 20, 15));
    // commandParameterMap["lidar_range_filter"].append(parameter("test" + QString("lidar_range_filter"), 0, 30, 15));
    // commandParameterMap["lidar_angular_filter"].append(parameter("test" + QString("lidar_angular_filter"), 0, 10, 5));
    // commandParameterMap["lane_detection"].append(parameter("test" + QString("lane_detection"), 1, 10, 6));
    // commandParameterMap["pothole_detection"].append(parameter("test" + QString("pothole_detection"), 3, 12, 7));
    // // commandParameterMap["nav"].append(parameter("test" + QString("nav"), 0, 5, 3));
    // commandParameterMap["pathfinder"].append(parameter("test" + QString("pathfinder"), 10, 30, 20));
    // commandParameterMap["gps"].append(parameter("test" + QString("gps"), 5, 15, 10));
    // commandParameterMap["go"].append(parameter("test" + QString("go"), 1, 5, 3));



    //connecting each checkbox to their processes.
    for(int i = 0;i < checkBoxes.size(); ++i){
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


    createSpoiler(label, commandParameterMap[label]);

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
            Spoiler* spoiler = qobject_cast<Spoiler*>(widget);
            if (spoiler!=nullptr) {
                if(spoiler->toggleButton.text()==label){
                    m_ui->runningScriptsList->takeAt(i);
                    // qDebug()<<widget<<sizeof(*widget);
                    // delete widget;
                    // qDebug()<<item<<sizeof(*widget);
                    // delete item;
                    // qDebug()<<spoiler<<sizeof(*spoiler);
                    delete spoiler;
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
void ExecBox::ReadJSON(){

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

void ExecBox::ReadYAML(){
    try {
        YAML::Node config = YAML::LoadFile("../../config.yaml");
        for(const auto& command : config){
            QString command_label = QString::fromStdString(command.first.as<std::string>());
            qDebug()<<command_label;
            for(const auto& param:command.second){
                QString param_label = QString::fromStdString(param.first.as<std::string>());
                double initial_value = param.second.as<double>();
                qDebug()<<param_label<<initial_value;
                commandParameterMap[command_label].append(parameter(param_label, 0, 100, initial_value));
            }
        }

    } catch (const YAML::ParserException& ex) {
        qCritical() << "Error parsing YAML:" << ex.what();
    } catch (const YAML::BadFile& ex) {
        qCritical() << "Error reading YAML file:" << ex.what();
    } catch (const YAML::Exception& ex) {
        qCritical() << "YAML Error:" << ex.what();
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

void ExecBox::createSpoiler(const QString command_label, QVector<parameter> parameters){
    Spoiler *spoiler = new Spoiler(command_label);
    qDebug()<<spoiler<<sizeof(*spoiler);

    // Add content to the spoiler
    QVBoxLayout *spoilerContent = new QVBoxLayout();
    if (parameters.isEmpty()) {
        QLabel *emptyLabel = new QLabel("No parameters", this);
        emptyLabel->setStyleSheet("font-style: italic; color: black;");
        spoilerContent->addWidget(emptyLabel);
    }

    qDebug()<<spoilerContent<<sizeof(spoilerContent);

    for(auto& param: parameters){
        QSlider *slider = new QSlider(Qt::Horizontal, this);
        qDebug()<<param.label<<param.min<<param.max<<param.initial;
        double scaleFactor = 100;
        // Set the range of the slider
        slider->setMinimum(int(param.min * scaleFactor));   // minimum value
        slider->setMaximum(int(param.max * scaleFactor)); // maximum value
        slider->setValue(int(param.initial * scaleFactor));    // initial value
        slider->setTickPosition(QSlider::TicksBelow); // show ticks below the slider
        slider->setTickInterval(1);  // interval between ticks
        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);


        // Create a label to display the slider value
        QLabel *param_name_with_value = new QLabel(param.label + ": " + QString::number(static_cast<double>(param.initial), 'f', static_cast<int>(std::log10(scaleFactor))), this);

        // Connect the slider's valueChanged signal to update the label
        connect(slider, &QSlider::valueChanged, this, [param_name_with_value, param, scaleFactor, command_label, this](int value) mutable {
            double realVal = static_cast<double>(value) / scaleFactor;
            param_name_with_value->setText(param.label + ": " + QString::number(realVal, 'f', static_cast<int>(std::log10(scaleFactor))));
            param.initial = realVal;
            writeInYAML(command_label, param);
        });
        param_name_with_value->setStyleSheet("QLabel {"
                             "font-size: 16px;"
                             "color: #000000;"
                             "}");
        spoilerContent->addWidget(param_name_with_value);

        spoilerContent->addWidget(slider);
    }
    spoiler->setContentLayout(*spoilerContent);

    connect(&spoiler->playButton, &QToolButton::clicked, this, [=]() {
        currentSelectedLabel = command_label;
        lastShownOutput.clear();
        m_ui->TerminalDisplay->clear();
        m_ui->TerminalDisplay->appendPlainText(ScriptOutputMap.value(currentSelectedLabel));
        qDebug() << "Button clicked:" << command_label;
    });


    // Add spoiler to layout
    m_ui->runningScriptsList->addWidget(spoiler);
    m_ui->runningScriptsList->addStretch();
}

void ExecBox::writeInYAML(QString command, parameter param){
    YAML::Node root;
    std::ifstream fin("../../config.yaml");
    if (fin.is_open()) {
        // Parse the existing YAML file
        root = YAML::Load(fin);
        fin.close();
    } else {
        qWarning() << "config.yaml not found. Creating a new one.";
    }
    std::string std_command = command.toStdString();
    std::string std_param_label = param.label.toStdString();
    root[std_command][std_param_label] = param.initial;
    YAML::Emitter emitter;
    emitter << root;
    std::ofstream fout("../../config.yaml");
    fout << emitter.c_str();
    fout.close();

    qDebug()<<"YAML data written to config.yaml";
}
