# IGVC_UI

A dynamic, user-friendly interface for managing and monitoring autonomous vehicle scripts and configurations.



## 🧠 Overview

**IGVC_UI** is a Qt-based GUI developed for the IGVC (Intelligent Ground Vehicle Competition) stack under Abhiyaan, IIT Madras. It enables users to:

- Launch and monitor multiple ROS-based scripts directly through GUI
- Configure parameters from YAML/JSON files
- View terminal outputs in real-time
- Interact with checkboxes mapped to script toggles
- Automatically handle user config files
- Customize parameters through sliders and dropdowns


## 🛠️ Build Instructions

> **Note**: Make sure you have Qt 5.15.3 (or compatible) and CMake 3.16+ installed.

```bash
git clone https://github.com/Mahmood-Sinan/IGVC_UI.git
cd IGVC_UI
mkdir build && cd build
cmake ..
make
```

Alternatively, open the project in **Qt Creator**, configure it with your desired Kit, and hit Build.

### Configuration

Before running the app, copy the example config files to your home directory and customize:

```bash
mkdir -p ~/.config/config_igvc_ui
cp config.json.example ~/.config/config_igvc_ui/config.json
cp config.yaml.example ~/.config/config_igvc_ui/config.yaml
```

Modify parameters as per your requirements.


## 🚀 Releases

Pre-built executable versions are available in the Releases section for quick setup and testing.

## 👥 Contributors

- [Mahmood Sinan](https://github.com/Mahmood-Sinan)
- [Anoushka Kuriakos](https://github.com/AnoushkaKuriakos)

### 💬 Questions?

For help or queries, feel free to reach out to the contributors or raise an issue in the repository.
