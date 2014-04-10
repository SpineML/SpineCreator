#-------------------------------------------------
#
# Project created by QtCreator 2011-12-06T16:35:55
#
#-------------------------------------------------

VPATH += ../shared
INCLUDEPATH += ../shared


QT       += core gui opengl xml network

greaterThan(QT_MAJOR_VERSION, 4) {
QT       += printsupport
}

TARGET = spinecreator
TEMPLATE = app

SOURCES += main.cpp \
        mainwindow.cpp \
    glwidget.cpp \
    population.cpp \
    rootdata.cpp \
    rootlayout.cpp \
    nineML_classes.cpp \
    projections.cpp \
    connection.cpp \
    connectionmodel.cpp \
    glconnectionwidget.cpp \
    nineml_layout_classes.cpp \
    cinterpreter.cpp \
    systemobject.cpp \
    layoutaliaseditdialog.cpp \
    layouteditpreviewdialog.cpp \
    vectormodel.cpp \
    valuelistdialog.cpp \
    genericinput.cpp \
    connectionlistdialog.cpp \
    experiment.cpp \
    editsimulators.cpp \
    propertiesmanager.cpp \
    nineml_graphicsitems.cpp \
    nineml_alscene.cpp \
    gvitems.cpp \
    grouptextitems.cpp \
    exportimage.cpp \
    dotwriter.cpp \
    regimegraphicsitem.cpp \
    systemmodel.cpp \
    undocommands.cpp \
    nineml_rootcomponentitem.cpp \
    nineml_alview.cpp \
    versionchange_dialog.cpp \
    savenetworkimage_dialog.cpp \
    generate_dialog.cpp \
    versioncontrol.cpp \
    commitdialog.cpp \
    viewVZlayoutedithandler.cpp \
    viewGVpropertieslayout.cpp \
    viewELexptpanelhandler.cpp \
    qcustomplot.cpp \
    logdata.cpp \
    aboutdialog.cpp \
    projectobject.cpp \
    filteroutundoredoevents.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    population.h \
    rootdata.h \
    nineML_classes.h \
    projections.h \
    connection.h \
    connectionmodel.h \
    glconnectionwidget.h \
    globalHeader.h \
    nineml_layout_classes.h \
    cinterpreter.h \
    systemobject.h \
    layoutaliaseditdialog.h \
    layouteditpreviewdialog.h \
    projections.h \
    vectormodel.h \
    valuelistdialog.h \
    genericinput.h \
    connectionlistdialog.h \
    experiment.h \
    editsimulators.h \
    propertiesmanager.h \
    nineml_graphicsitems.h \
    nineml_alscene.h \
    gvitems.h \
    grouptextitems.h \
    exportimage.h \
    dotwriter.h \
    regimegraphicsitem.h \
    systemmodel.h \
    undocommands.h \
    nineml_rootcomponentitem.h \
    nineml_alview.h \
    versionchange_dialog.h \
    savenetworkimage_dialog.h \
    generate_dialog.h \
    versioncontrol.h \
    commitdialog.h \
    viewVZlayoutedithandler.h \
    viewGVpropertieslayout.h \
    viewELexptpanelhandler.h \
    qcustomplot.h \
    logdata.h \
    aboutdialog.h \
    projectobject.h \
    rootlayout.h \
    filteroutundoredoevents.h

FORMS    += mainwindow.ui \
    ninemlsortingdialog.ui \
    valuelistdialog.ui \
    brahms_dialog.ui \
    connectionlistdialog.ui \
    editsimulators.ui \
    exportimage.ui \
    versionchange_dialog.ui \
    savenetworkimage_dialog.ui \
    generate_dialog.ui \
    commitdialog.ui \
    aboutdialog.ui

RESOURCES += \
    icons.qrc

win32:release{
    LIBS += "-Lc:/Program Files/Graphviz2.26.3/lib/release/lib" -lgvc -lgraph -lGLU32
    INCLUDEPATH += "c:/Program Files/Graphviz2.26.3/include"
    DEPENDPATH += "c:/Program Files/Graphviz2.26.3/lib/release/lib"
}
win32:debug{
    LIBS += "-Lc:/Program Files/Graphviz2.26.3/lib/debug/lib" -lgvc -lgraph -lGLU32
    INCLUDEPATH += "c:/Program Files/Graphviz2.26.3/include"
    DEPENDPATH += "c:/Program Files/Graphviz2.26.3/lib/debug/lib"
}
linux-g++{
    LIBS += -L/usr/lib/graphviz -lgvc -lgraph -lGLU -lpython2.7
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += /usr/include/graphviz
    DEPENDPATH += /usr/lib/graphviz
}
linux-g++-64{
    LIBS += -L/usr/lib/graphviz -lgvc -lgraph -lGLU -lpython2.7
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += /usr/include/graphviz
    DEPENDPATH += /usr/lib/graphviz
}
macx{
    LIBS += -L/usr/local/lib/ -L/usr/local/lib/graphviz/ -lpython -lgvc -lgraph
    INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6
    INCLUDEPATH += /usr/local/include/graphviz
    INCLUDEPATH += /usr/local/include
    INCLUDEPATH += /usr/include/graphviz
    DEPENDPATH += /usr/local/lib/graphviz
    DEPENDPATH += /usr/local/lib
}

OTHER_FILES += \
    neuralNetworks.pro.user

target.path = /usr/bin
INSTALLS += target
