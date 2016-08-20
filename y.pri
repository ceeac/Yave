CONFIG += c++1z

LIBS += -lpthread

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += $$PWD/y/*.cpp \
		   $$PWD/y/test/*.cpp

HEADERS += $$PWD/y/*.h \
		   $$PWD/y/core/*.h \
		   $$PWD/y/test/*.h

win32:DEFINES += WIN32

ALLWFLAGS = -Wconversion -Weffc++ -Wshadow
WFLAGS = -pedantic -Wall -Wextra -Woverloaded-virtual -Wfloat-equal -Wwrite-strings -Wpointer-arith -Wcast-qual -Wcast-align -Wunreachable-code -Wold-style-cast

QMAKE_CXXFLAGS += $$WFLAGS
QMAKE_LFLAGS += $$WFLAGS

QMAKE_CXXFLAGS += -pedantic

CONFIG(debug, debug|release) {
} else {
	QMAKE_CXXFLAGS += -O3 -flto
	QMAKE_LFLAGS += -O3 -flto
}
