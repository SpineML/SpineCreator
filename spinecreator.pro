#-------------------------------------------------
#
# Project created by QtCreator 2011-12-06T16:35:55
#
#-------------------------------------------------

VPATH += ../shared
INCLUDEPATH += ../shared

QT += core gui opengl xml network svg

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += printsupport
}

TARGET = spinecreator
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    filteroutundoredoevents.cpp \
    CL_classes.cpp \
    SC_utilities.cpp \
    NL_population.cpp \
    SC_projectobject.cpp \
    SC_aboutdialog.cpp \
    SC_commitdialog.cpp \
    NL_connection.cpp \
    NL_projection_and_synapse.cpp \
    SC_settings.cpp \
    EL_experiment.cpp \
    SC_systemmodel.cpp \
    NL_systemobject.cpp \
    SC_undocommands.cpp \
    SC_valuelistdialog.cpp \
    SC_vectorlistmodel.cpp \
    SC_vectormodel.cpp \
    SC_versioncontrol.cpp \
    SC_network_layer_rootdata.cpp \
    SC_network_layer_rootlayout.cpp \
    SC_connectionlistdialog.cpp \
    SC_connectionmodel.cpp \
    SC_dotwriter.cpp \
    SC_export_component_image.cpp \
    SC_export_network_image.cpp \
    NL_genericinput.cpp \
    SC_python_connection_generate_dialog.cpp \
    SC_logged_data.cpp \
    SC_component_scene.cpp \
    SC_component_view.cpp \
    SC_component_propertiesmanager.cpp \
    SC_component_graphicsitems.cpp \
    CL_layout_classes.cpp \
    SC_layout_aliaseditdialog.cpp \
    SC_layout_editpreviewdialog.cpp \
    SC_component_grouptextitems.cpp \
    SC_component_gvitems.cpp \
    SC_component_rootcomponentitem.cpp \
    SC_viewELexptpanelhandler.cpp \
    SC_viewGVpropertieslayout.cpp \
    SC_viewVZlayoutedithandler.cpp \
    SC_layout_cinterpreter.cpp \
    SC_network_2d_visualiser_panel.cpp \
    SC_network_3d_visualiser_panel.cpp

HEADERS += mainwindow.h \
    globalHeader.h \
    qcustomplot.h \
    filteroutundoredoevents.h \
    qmessageboxresizable.h \
    CL_classes.h \
    SC_utilities.h \
    NL_population.h \
    SC_projectobject.h \
    SC_aboutdialog.h \
    SC_commitdialog.h \
    NL_connection.h \
    NL_projection_and_synapse.h \
    SC_settings.h \
    EL_experiment.h \
    SC_systemmodel.h \
    NL_systemobject.h \
    SC_undocommands.h \
    SC_valuelistdialog.h \
    SC_vectorlistmodel.h \
    SC_vectormodel.h \
    SC_versioncontrol.h \
    SC_network_layer_rootdata.h \
    SC_network_layer_rootlayout.h \
    SC_connectionlistdialog.h \
    SC_connectionmodel.h \
    SC_dotwriter.h \
    SC_export_component_image.h \
    SC_export_network_image.h \
    NL_genericinput.h \
    SC_python_connection_generate_dialog.h \
    SC_logged_data.h \
    SC_component_scene.h \
    SC_component_view.h \
    SC_component_propertiesmanager.h \
    SC_component_graphicsitems.h \
    CL_layout_classes.h \
    SC_layout_aliaseditdialog.h \
    SC_layout_editpreviewdialog.h \
    SC_component_gvitems.h \
    SC_component_grouptextitems.h \
    SC_component_rootcomponentitem.h \
    SC_viewELexptpanelhandler.h \
    SC_viewGVpropertieslayout.h \
    SC_viewVZlayoutedithandler.h \
    SC_layout_cinterpreter.h \
    SC_network_2d_visualiser_panel.h \
    SC_network_3d_visualiser_panel.h

FORMS += mainwindow.ui \
    valuelistdialog.ui \
    connectionlistdialog.ui \
    generate_dialog.ui \
    commitdialog.ui \
    aboutdialog.ui \
    settings_window.ui \
    export_component_image.ui \
    export_network_image.ui

RESOURCES += icons.qrc

win32:release {
    DEFINES += _MATH_DEFINES_DEFINED
    LIBS += "-LC:\SDKs\Graphviz232\lib\release\lib" "-LC:\Python27\libs" -lGLU32 -lpython27
    INCLUDEPATH += "C:\SDKs\Graphviz232\include"
    INCLUDEPATH += "C:\Python27\include"
    DEPENDPATH += "C:\SDKs\Graphviz232\lib\release\lib"
    DEPENDPATH += "C:\Python27\libs"
    DEPENDPATH += "C:\Python27"
}
win32:debug {
    DEFINES += _MATH_DEFINES_DEFINED
    LIBS += "-LC:\SDKs\Graphviz232\lib\debug\lib" "-LC:\Python27\libs" -lGLU32 -lpython27
    INCLUDEPATH += "C:\SDKs\Graphviz232\include"
    INCLUDEPATH += "C:\Python27\include"
    DEPENDPATH += "C:\SDKs\Graphviz232\lib\debug\lib"
    DEPENDPATH += "C:\Python27\libs"
    DEPENDPATH += "C:\Python27"
}
linux-g++ {
    LIBS += -L/usr/lib/graphviz -L/opt/graphviz/lib -lGLU -lpython2.7
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += /usr/include/graphviz /opt/graphviz/include
    DEPENDPATH += /usr/lib/graphviz
}
linux-g++-64 {
    LIBS += -L/usr/lib/graphviz -L/opt/graphviz/lib -lGLU -lpython2.7
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += /usr/include/graphviz /opt/graphviz/include
    DEPENDPATH += /usr/lib/graphviz
}
macx {
    QMAKE_MAC_SDK = macosx
    QMAKE_CXXFLAGS += -O0 -g
    LIBS +=  -L/opt/local/lib/graphviz/ -lpython2.7 # -L/opt/local/lib/ # this causes libJPEG.dylib conflict and is not required.
    INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6
    INCLUDEPATH += /opt/local/include /opt/local/include/graphviz
    DEPENDPATH +=  /opt/local/lib/graphviz
}
linux {
    QMAKE_CXXFLAGS += -Wall

    # Installation stuff for Linux. Important for debian builds
    documentation.path = /usr/share/man/man1
    documentation.files = spinecreator.1

    icons.path = /usr/share/pixmaps
    icons.files = spinecreator.png spinecreator.xpm

    desktop.path = /usr/share/applications
    desktop.files = spinecreator.desktop

    INSTALLS += documentation icons desktop
}

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

OTHER_FILES += spinecreator.pro.user

target.path = /usr/bin
INSTALLS += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
