#include "execbox.h"
#include "spoiler.h"

void ExecBox::createConfigDir(QString configDirPath)
{
    //create this directory if it does not exists
    QDir configDir(configDirPath);
    if (!configDir.exists()) {
        bool created = configDir.mkpath(".");  // create all missing parent dirs too
        if (!created) {
            qDebug() << "Failed to create config directory:" << configDirPath;
        }
    }
}

ExecBox::ExecBox(QWidget *parent,Ui::MainWindow* ui) : QWidget(parent), m_ui(ui) {

    QString configDirPath = QDir::homePath() + "/.config/config_igvc_ui/";

    createConfigDir(configDirPath);

    // Set the full paths to your config files inside that directory
    JSONpath = configDirPath + "config.json";
    YAMLpath = configDirPath + "config.yaml";

    //reading json file
    ReadJSON();
    ReadYAML();

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

    //timer to update terminal display
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

    //to create a spoiler when the process starts
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
                    delete spoiler;
                    break;
                }
            }
        }
    }
    ScriptOutputMap.remove(label);
    update();

    //The spoiler taken from the running list should be converted to this type to do anything on it
    // qobject_cast<Spoiler*>(m_ui->runningScriptsList->widget())
}

// this function reads the json file
void ExecBox::ReadJSON(){

    QFile File(JSONpath);

    //if there is no JSON file found in the JSONpath, the programme creates a default one
    if (!File.exists()) {
        qDebug() << "config.json not found, creating default file...";

        QJsonObject defaultObject;
        defaultObject["working_directory"] = "";

        QJsonArray commandArray;
        QJsonObject commandEntry;
        commandEntry["command"] = "";
        commandEntry["label"] = "";
        commandArray.append(commandEntry);
        defaultObject["commands"] = commandArray;

        QJsonArray defaultsArray;
        defaultsArray.append("");
        defaultObject["defaults"] = defaultsArray;

        QJsonDocument doc(defaultObject);

        if (File.open(QIODevice::WriteOnly | QIODevice::Text)) {
            File.write(doc.toJson(QJsonDocument::Indented));
            File.close();
            qDebug() << "Created default JSON file at:"<< JSONpath;
        } else {
            qDebug() << "Failed to create config.json.";
            return;
        }
    }

    //Reads the JSON file
    QByteArray Bytes;
    if(File.open(QIODevice::ReadOnly | QIODevice::Text)){

        //Extract the JSON data to ByteArray
        Bytes = File.readAll();
        File.close();

        //convert the ByteArray to JsonDocument and check for errors
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

        //Convert the JSONDocument to JsonObject
        QJsonObject rootObj = Document.object();

        //Extract working directory
        working_directory = rootObj["working_directory"].toString();
        qDebug()<<"Current working directory:"<<working_directory;

        if(!rootObj["commands"].isArray()){
            qDebug()<<"Expected 'commands' to be an array";
            return;
        }

        //Extract commands array
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
            label_checkbox_map[label]=checkbox;

            SetupUI(checkbox);
            CheckBoxProcessMap[checkbox] = nullptr;
            checkbox = nullptr;
        }

        //reading defaults in the json file and adding them to defaults list
        QJsonArray defaults_json_array = rootObj["defaults"].toArray();
        for(int i = 0; i < defaults_json_array.size(); i++){
            QString current_default = defaults_json_array.at(i).toString();
            if(label_checkbox_map.contains(current_default)){
                defaults.append(label_checkbox_map[current_default]);
            }
            else{
                qDebug()<<current_default<<"not found";
            }
        }
    }
    add_default_buttons();
}

void ExecBox::ReadYAML(){

    //If  no YAML file found in the YAMLpath, the programme creates a default one
    if (!QFile::exists(YAMLpath)) {
        // Create a default YAML template
        QFile file(YAMLpath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "parameters:\n";
            out << "limits:\n";
            file.close();
            qDebug() << "Created default YAML file at:" << YAMLpath;
        } else {
            qCritical() << "Could not create default YAML file at:" << YAMLpath;
            return;
        }
    }

    //reading the YAML file
    try {
        YAML::Node config = YAML::LoadFile(YAMLpath.toStdString());
        YAML::Node parameters = config["parameters"];
        for(const auto& command : parameters){
            QString command_label = QString::fromStdString(command.first.as<std::string>());

            for(const auto& param : command.second){
                QString param_label = QString::fromStdString(param.first.as<std::string>());
                double initial_value = param.second.as<double>();
                double min = config["limits"][command_label.toStdString()][param_label.toStdString()]["min"].as<double>();
                double max = config["limits"][command_label.toStdString()][param_label.toStdString()]["max"].as<double>();
                commandParameterMap[command_label].append(Parameter(param_label, min, max, initial_value));
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
    m_ui->buttons->setAlignment(Qt::AlignTop);

    QPixmap pixmap(":/assets/abhiyaan_logo.png");

    m_ui->LogoArea->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_ui->LogoArea->setPixmap(
        pixmap.scaled(205, 70, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
        );

    update();
}

void ExecBox::createSpoiler(const QString command_label, QVector<Parameter> parameters){
    Spoiler *spoiler = new Spoiler(command_label, 300, this);

    // Add content to the spoiler
    QVBoxLayout *spoilerContent = new QVBoxLayout(this);
    if (parameters.isEmpty()) {
        QLabel *emptyLabel = new QLabel("No parameters", this);
        emptyLabel->setStyleSheet("font-style: italic; color: black;");
        spoilerContent->addWidget(emptyLabel);
    }

    for(auto& param: parameters){
        QSlider *slider = new QSlider(Qt::Horizontal, this);
        double scaleFactor = 1000; //how much decimal places u want to be visible, put here 10 power that value

        // Set the range of the slider
        slider->setMinimum(int(param.min * scaleFactor));   // minimum value
        slider->setMaximum(int(param.max * scaleFactor)); // maximum value
        slider->setValue(int(param.initial * scaleFactor));    // initial value
        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

        QString fullParamName = command_label + "/" + param.label; // create unique key
        lastParamValues[fullParamName] = param.initial;            // store the initial value


        // Create a label to display the slider value
        QLabel *param_name_with_value = new QLabel(param.label + ": " + QString::number(static_cast<double>(param.initial), 'f', static_cast<int>(std::log10(scaleFactor))), this);

        // Connect the slider's valueChanged signal to update the label

        // connect(slider, &QSlider::valueChanged, this, [param_name_with_value, param, scaleFactor, command_label, this, &slider](int value) mutable{  //This line also works
        connect(slider, &QSlider::valueChanged, this, [=](int value) mutable{
            double realVal = static_cast<double>(value)/scaleFactor;
            QString fullParamName = command_label + "/" + param.label;

            if (command_label == "lane_detection")
            {   double otherVal = getOtherParamValue(command_label, (param.label == "low") ? "high" : "low");

                if ((param.label == "low" && realVal > otherVal) || (param.label == "high" && realVal < otherVal))
                {   QSignalBlocker blocker(slider);
                    slider->setValue(static_cast<int>(lastParamValues[fullParamName] * scaleFactor));
                    return;}
            }

            param_name_with_value->setText(param.label + ": " + QString::number(realVal, 'f', static_cast<int>(std::log10(scaleFactor))));

            for (auto& p : commandParameterMap[command_label]) {
                if (p.label == param.label)
                {   p.initial = realVal; // Modify the original parameter
                    break;
                }
            }

// Here a copy of param it created inside lambda and it's value is being changed. And for writeInYAML that copy is being passed
            param.initial = realVal;
            writeInYAML(command_label, param);
            lastParamValues[fullParamName] = realVal;

            // Uncomment this code snippet to show the data in commandParameterMap in terminal
/*
            for(auto it = commandParameterMap.begin();it!= commandParameterMap.end();it++){
                for(auto param: it.value()){
                    qDebug()<<param.label<<param.initial;
                }
            }
*/
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
        // qDebug() << "Button clicked:" << command_label;
    });


    // Add spoiler to layout
    m_ui->runningScriptsList->setAlignment(Qt::AlignTop);
    m_ui->runningScriptsList->addWidget(spoiler);
}

double ExecBox::getOtherParamValue(QString command_label, QString other_param_name)
{
    for (const auto& p : commandParameterMap[command_label])
    {   if (p.label == other_param_name)
        {   return p.initial;}
    }

    qWarning() << "Warning: Param" << other_param_name << "not found under" << command_label;
    return 0.0; // fallback default
}

void ExecBox::writeInYAML(QString command, Parameter param){

    //Starts a YAML node
    YAML::Node root;

    //Reads the YAML file in read only mode
    std::ifstream fin(YAMLpath.toStdString());

    if (fin.is_open()) {

        // Parse the existing YAML file
        root = YAML::Load(fin);
        fin.close();
    } else {
        qWarning() << "config.yaml not found. Creating a new one.";
    }

    // Variables to get the command label and parameter label in std::string
    std::string std_command = command.toStdString();
    std::string std_param_label = param.label.toStdString();

    //rounding parameter initial value
    double rounded = std::round(param.initial * 1000.0) / 1000.0;
    root["parameters"][std_command][std_param_label] = rounded;

    //Storing the limits map in a YAML Node
    YAML::Node limits = root["limits"];

    // Emitter to write data into YAML
    YAML::Emitter emitter;

    // Begins Whole YAML
    emitter << YAML::BeginMap;

    // Begins Parameter Map
    emitter<<YAML::Key<< "parameters"<<YAML::Value;

    // Begins the nested map in parameters
    emitter << YAML::BeginMap;

    // Stores each command label and parameters in emitter
    for (const auto& command : root["parameters"]) {

        // Writes each command label and leaves the key for that label
        emitter << YAML::Key << command.first.as<std::string>() << YAML::Value;
        //In the key for each label begins a map
        emitter << YAML::BeginMap;

        //Stores the parameters under each command label
        for (const auto& param : command.second) {
            double value = param.second.as<double>();
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(3) << value;

            //For each parameter we starts a key value pair and adds it to emitter
            emitter << YAML::Key << param.first.as<std::string>() << YAML::Value << ss.str();
        }

        // Ends the map for command label
        emitter << YAML::EndMap;
    }

    //ends the parameter map
    emitter << YAML::EndMap;

    // begins the limits map with "limits" as key and leaves a blank value
    emitter << YAML::Key << "limits" << YAML::Value;

    // begins the value of "limits" as a map
    emitter << YAML::BeginMap;

    // Stores each value of commands in limits
    for (const auto& command : limits) {

        //begins each command label as key and leaves a blank value
        emitter << YAML::Key << command.first.as<std::string>() << YAML::Value;

        // begins the value of each command label as a map
        emitter << YAML::BeginMap;

        // stores parameters
        for (const auto& param : command.second) {

            // begins the parameter parameter label as a key and leaves a blank value
            emitter << YAML::Key << param.first.as<std::string>() << YAML::Value;

            //begins the value of parameter label as a map
            emitter <<YAML::BeginMap;

            // adds key value pairs of min and max in the map which is the value of each parameter label
            double min = param.second["min"].as<double>();

            std::ostringstream ss;

            ss << std::fixed << std::setprecision(3) << min;
            emitter << YAML::Key << "min" << YAML::Value << ss.str();
            double max = param.second["max"].as<double>();

            ss.str("");
            ss.clear();

            ss << std::fixed << std::setprecision(3) << max;
            emitter << YAML::Key << "max" << YAML::Value << ss.str();

            // ends the map with key as each parameter label
            emitter << YAML::EndMap;
        }

        // ends the map with key as each command label
        emitter << YAML::EndMap;
    }

    // ends the "limits" map
    emitter << YAML::EndMap;

    // opens the yaml file in write mode
    std::ofstream fout(YAMLpath.toStdString());

    // convert the emitter to c string and writes into yaml
    fout << emitter.c_str();

    // closes the yaml file
    fout.close();

    // qDebug()<<"YAML data written to config.yaml";
}

void ExecBox::add_default_buttons(){

    m_ui->buttons->addSpacing(20);

    set_default = new QPushButton(this);
    select_default = new QPushButton(this);

    set_default->setText("set default");
    select_default->setText("select default");

    set_default->setStyleSheet(R"(
        QPushButton {
            background-color: #0078D7;
            color: white;
            padding: 8px 16px;
            border: none;
            border-radius: 6px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #005A9E;
        }
        QPushButton:pressed {
            background-color: #004578;
        }
    )");

    select_default->setStyleSheet(R"(
        QPushButton {
            background-color: #28a745;
            color: white;
            padding: 8px 16px;
            border: none;
            border-radius: 6px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #218838;
        }
        QPushButton:pressed {
            background-color: #1e7e34;
        }
    )");

    QHBoxLayout* default_layout = new QHBoxLayout;

    default_layout->addWidget(set_default);
    default_layout->addWidget(select_default);

    m_ui->buttons->addLayout(default_layout);

    //When set default is clicked, the defaults list is cleared and updated with new ones.
    connect(set_default, &QPushButton::clicked, this, [this]() {
        defaults.clear();
        for(auto it = checkBoxes.begin(); it < checkBoxes.end(); it++){
            if((*it)->isChecked()){
                defaults.push_back(*it);
            }
        }
        QFile file(JSONpath);

        if (file.open(QIODevice::ReadWrite)) {

            // Read the existing content
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);

            if (doc.isObject()) {
                QJsonObject jsonObject = doc.object();

                list_of_labels_of_defaults.clear();

                // Add the labels of the selected checkboxes to the "list_of_labels_of_defaults" list
                for (int i = 0; i < defaults.size(); ++i) {
                    if (defaults[i]->isChecked()) {
                        list_of_labels_of_defaults.append(defaults[i]->text());
                    }
                }

                // Set the "defaults" field with the selected command labels
                jsonObject["defaults"] = QJsonArray::fromStringList(list_of_labels_of_defaults);

                // Write the updated JSON back to the file
                file.resize(0);  // Clear the existing content of the file
                file.write(QJsonDocument(jsonObject).toJson());
                file.close();

                qDebug() << "Defaults updated successfully!";
            } else {
                qDebug() << "Invalid JSON format!";
            }
        } else {
            qDebug() << "Error opening file!";
        }
    });

    //When select default is clicked, all the current checkboxes are unchecked and default ones are checked
    connect(select_default, &QPushButton::clicked, this, [this]() {
        for(auto it = checkBoxes.begin(); it < checkBoxes.end(); it++){
            (*it)->setChecked(false);
        }
        for(auto it = defaults.begin(); it < defaults.end(); it++){
            (*it)->setChecked(true);
        }
    });
}
