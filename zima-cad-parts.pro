# -------------------------------------------------
# Project created by QtCreator 2009-05-18T21:26:48
# -------------------------------------------------
QT += network webkit opengl
TARGET = ZIMA-CAD-Parts
TEMPLATE = app

win32:INCLUDEPATH += ../
VPATH += ./src

INCLUDEPATH += libqdxf/src
INCLUDEPATH += libqdxf/libdxfrw/src

LIBS += -lpoppler-qt4

SOURCES += zima-cad-parts.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    serversmodel.cpp \
    item.cpp \
    filemodel.cpp \
    basedatasource.cpp \
    baseremotedatasource.cpp \
    ftpdatasource.cpp \
    localdatasource.cpp \
    addeditdatasource.cpp \
    downloadmodel.cpp \
    src/extensions/productview/productview.cpp \
    src/filtersdialog.cpp \
    src/metadata.cpp \
    src/techspecswebview.cpp \
    src/zimautils.cpp \
    src/errordialog.cpp \
    src/treeautodescent.cpp \
    src/thumbnail.cpp \
    src/filefiltermodel.cpp \
    src/filefilters/filefilter.cpp \
    src/filefilters/extensionfilter.cpp \
    src/filefilters/filtergroup.cpp \
    src/filefilters/versionfilter.cpp \
    src/datatransfer.cpp \
    src/transferhandler.cpp \
    src/extensions/productview/abstractproductview.cpp \
    src/extensions/productview/proeproductview.cpp \
    src/extensions/productview/dxfproductview.cpp \
    libqdxf/libdxfrw/src/drw_base.h \
    libqdxf/libdxfrw/src/drw_entities.cpp \
    libqdxf/libdxfrw/src/drw_entities.h \
    libqdxf/libdxfrw/src/drw_interface.h \
    libqdxf/libdxfrw/src/drw_objects.cpp \
    libqdxf/libdxfrw/src/drw_objects.h \
    libqdxf/libdxfrw/src/libdxfrw.cpp \
    libqdxf/libdxfrw/src/libdxfrw.h \
    libqdxf/libdxfrw/src/intern/drw_cptable932.h \
    libqdxf/libdxfrw/src/intern/drw_cptable936.h \
    libqdxf/libdxfrw/src/intern/drw_cptable949.h \
    libqdxf/libdxfrw/src/intern/drw_cptable950.h \
    libqdxf/libdxfrw/src/intern/drw_cptables.h \
    libqdxf/libdxfrw/src/intern/drw_textcodec.cpp \
    libqdxf/libdxfrw/src/intern/drw_textcodec.h \
    libqdxf/libdxfrw/src/intern/dxfreader.cpp \
    libqdxf/libdxfrw/src/intern/dxfreader.h \
    libqdxf/libdxfrw/src/intern/dxfwriter.cpp \
    libqdxf/libdxfrw/src/intern/dxfwriter.h \
    libqdxf/src/dxfinterface.cpp \
    libqdxf/src/dxfsceneview.cpp \
    libqdxf/src/mtexttohtml.cpp \
    libqdxf/src/spline.cpp \
    src/extensions/productview/pdfproductview.cpp \
    src/extensions/productview/failbackproductview.cpp \
    src/serverswidget.cpp


HEADERS += mainwindow.h \
    settingsdialog.h \
    serversmodel.h \
    item.h \
    filemodel.h \
    basedatasource.h \
    baseremotedatasource.h \
    ftpdatasource.h \
    localdatasource.h \
    addeditdatasource.h \
    downloadmodel.h \
    src/extensions/productview/productview.h \
    src/filtersdialog.h \
    src/metadata.h \
    src/techspecswebview.h \
    src/zima-cad-parts.h \
    src/zimautils.h \
    src/errordialog.h \
    src/treeautodescent.h \
    src/thumbnail.h \
    src/filefiltermodel.h \
    src/filefilters/filefilter.h \
    src/filefilters/extensionfilter.h \
    src/filefilters/filtergroup.h \
    src/filefilters/versionfilter.h \
    src/datatransfer.h \
    src/transferhandler.h \
    src/extensions/productview/abstractproductview.h \
    src/extensions/productview/proeproductview.h \
    src/extensions/productview/dxfproductview.h \
    libqdxf/src/mtexttohtml.h \
    libqdxf/src/scene_items.h \
    libqdxf/src/dxfsceneview.h \
    libqdxf/src/dxfinterface.h \
    libqdxf/src/spline.h \
    src/extensions/productview/pdfproductview.h \
    src/extensions/productview/failbackproductview.h \
    src/serverswidget.h


FORMS += mainwindow.ui \
    settingsdialog.ui \
    addeditdatasource.ui \
    src/filtersdialog.ui \
    src/errordialog.ui \
    src/extensions/productview/proeproductview.ui \
    src/extensions/productview/productview.ui \
    src/extensions/productview/dxfproductview.ui \
    src/extensions/productview/pdfproductview.ui \
    src/extensions/productview/failbackproductview.ui
RESOURCES += zima-cad-parts.qrc

OTHER_FILES += \
    zima-cad-parts.rc \
    LICENSE \
    AUTHORS \
    data/zima-cad-parts.html \
    data/zima-cad-parts_cs_CZ.html \
    README \
    data/extensions/productview/proeproductview.html

TRANSLATIONS = locale/zima-cad-parts_cs_CZ.ts

win32:CONFIG += static
win32:RC_FILE = src/zima-cad-parts.rc

ICON = gfx/icon.icns
