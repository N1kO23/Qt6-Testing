QT += widgets

TEMPLATE = app
TARGET = hello

SOURCES += src/main.cpp

# Additional configuration for Windows to include the Qt platform plugin
win32: {
    CONFIG += qt
    QTPLUGIN += qwindows
}

# Additional configuration for macOS to use the Cocoa framework
macx: {
    CONFIG += qt
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12
    LIBS += -framework Cocoa
}

# Additional configuration for Linux to use the X11 platform plugin
unix: {
    CONFIG += qt
    QTPLUGIN += xcb
}

# Define the resource file(s) to include in the program
RESOURCES += main.qrc