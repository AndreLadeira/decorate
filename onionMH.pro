TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    cops/mkp.cpp \
    cops/sat.cpp \
    cops/tsp.cpp \
    lib/abstractvalues.cpp \
    lib/facilities.cpp \
    lib/loopcontroller.cpp \
    lib/parameters.cpp \
    lib/random.cpp \
    lib/recorder.cpp \
    lib/stddecorators.cpp \
    lib/timer.cpp \
    lib/values.cpp \
    mh/rrga.cpp \
    mh/sian.cpp \
    main.cpp \
    apps/std/tsp_min.cpp \
    apps/std/tsp_test.cpp \
    cops/bmkfcns.cpp \
    apps/std/bmf.cpp \
    apps/std/bmf_test.cpp

HEADERS += \
    apps/delta/tsp_min_d.h \
    apps/std/mkp_min.h \
    apps/std/tsp_full.h \
    apps/std/tsp_min.h \
    cops/mkp.h \
    cops/sat.h \
    cops/tsp.h \
    lib/accept.h \
    lib/algorithm.h \
    lib/create.h \
    lib/dataload.h \
    lib/facilities.h \
    lib/functor.h \
    lib/loopcontroller.h \
    lib/neighbor.h \
    lib/objective.h \
    lib/observer.h \
    lib/onion.h \
    lib/onionmh.h \
    lib/parameters.h \
    lib/random.h \
    lib/recorder.h \
    lib/stddecorators.h \
    lib/timer.h \
    lib/trackutil.h \
    lib/types.h \
    lib/update.h \
    lib/values.h \
    mh/rrga.h \
    mh/sian.h \
    test/dataloaders.h \
    garbage.h \
    apps/stagtest/tsp_st.h \
    apps/test_fcn.h \
    apps/std/test_fcn.h \
    apps/std/tsp_test.h \
    apps/std/mkp_test.h \
    cops/bmkfcns.h \
    apps/std/apps.h

CONFIG(debug, debug|release) {
    DEFINES += "__DEBUG__"
}
CONFIG(release, debug|release) {
    macx | linux: QMAKE_CXXFLAGS += -O3
    DEFINES -= "__DEBUG__"
}
