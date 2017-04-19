#include "SC_utilities.h"
#include <QSettings>

void
SCUtilities::storeError (QString emsg)
{
    QSettings settings;
    int num_errs = settings.beginReadArray("errors");
    settings.endArray();
    settings.beginWriteArray("errors");
    settings.setArrayIndex(num_errs + 1);
    settings.setValue("errorText", emsg);
    settings.endArray();
}
