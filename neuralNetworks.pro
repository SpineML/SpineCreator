#-------------------------------------------------
#
# Project created by QtCreator 2011-12-06T16:35:55
#
#-------------------------------------------------

VPATH += ../shared
INCLUDEPATH += ../shared

QT       += core gui opengl xml network svg

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
    filteroutundoredoevents.cpp \
    batchexperimentwindow.cpp \
    vectorlistmodel.cpp

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
    filteroutundoredoevents.h \
    batchexperimentwindow.h \
    vectorlistmodel.h \
    qmessageboxresizable.h

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
    aboutdialog.ui \
    batchexperimentwindow.ui

RESOURCES += \
    icons.qrc

# Use of the cgraph API from graphviz version 2.32 and above is the default. Can configure
# to build with the deprecated libgraph API, if required (for Debian 7 and older Linux
# distros). To do this, add "CONFIG+=use_libgraph_not_libcgraph" to the "Additional
# arguments" text box under "Build Steps" in the QtCreator project configuration, or call
# qmake like this: qmake "CONFIG+=use_libgraph_not_libcgraph"
CONFIG(use_libgraph_not_libcgraph) {
    DEFINES += USE_LIBGRAPH_NOT_LIBCGRAPH
    LIBS += -lgvc -lgraph
} else {
    LIBS += -lgvc -lcgraph
}

win32:release{
    LIBS += "-Lc:/Program Files/Graphviz2.26.3/lib/release/lib" -lGLU32
    INCLUDEPATH += "c:/Program Files/Graphviz2.26.3/include"
    DEPENDPATH += "c:/Program Files/Graphviz2.26.3/lib/release/lib"
}
win32:debug{
    LIBS += "-Lc:/Program Files/Graphviz2.26.3/lib/debug/lib" -lGLU32
    INCLUDEPATH += "c:/Program Files/Graphviz2.26.3/include"
    DEPENDPATH += "c:/Program Files/Graphviz2.26.3/lib/debug/lib"
}
linux-g++{
    LIBS += -L/usr/lib/graphviz -L/opt/graphviz/lib -lGLU -lpython2.7
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += /usr/include/graphviz /opt/graphviz/include /opt/graphviz/include/graphviz
    DEPENDPATH += /usr/lib/graphviz
}
linux-g++-64{
    LIBS += -L/usr/lib/graphviz -L/opt/graphviz/lib -lGLU -lpython2.7
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += /usr/include/graphviz /opt/graphviz/include /opt/graphviz/include/graphviz
    DEPENDPATH += /usr/lib/graphviz
}
macx{
    QMAKE_CXXFLAGS += -O0 -g
    LIBS += -L/opt/local/lib/ -L/opt/local/lib/graphviz/ -lpython
    INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6
    INCLUDEPATH += /opt/local/include /opt/local/include/graphviz
    DEPENDPATH +=  /opt/local/lib/graphviz
}

OTHER_FILES += \
neuralNetworks.pro.user

documentation.path = /usr/share/man/man1
documentation.files = spinecreator.1

icons.path = /usr/share/pixmaps
icons.files = spinecreator.png

desktop.path = /usr/share/applications
desktop.files = spinecreator.desktop

target.path = /usr/bin
INSTALLS += target documentation icons desktop

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
