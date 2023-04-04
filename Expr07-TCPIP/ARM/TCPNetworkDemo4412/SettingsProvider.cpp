#include "SettingsProvider.h"

QSettings SettingsContainer(ST_MAIN_DATABASE_PATH, QSettings::IniFormat); //Initialize settings container using INI format, for capability with embedded platforms.
