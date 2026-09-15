#!/bin/sh
set -eu
runtime=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
unset QT_PLUGIN_PATH QT_QPA_PLATFORM_PLUGIN_PATH QML_IMPORT_PATH QML2_IMPORT_PATH
export LD_LIBRARY_PATH="$runtime/lib"
export QTWEBENGINEPROCESS_PATH="$runtime/bin/QtWebEngineProcess"
export QTWEBENGINE_RESOURCES_PATH="$runtime/resources"
export QTWEBENGINE_LOCALES_PATH="$runtime/translations/qtwebengine_locales"
export CSF_ShadersDirectory="$runtime/occt/Shaders"
export CSF_SHMessage="$runtime/occt/SHMessage"
export CSF_XSMessage="$runtime/occt/XSMessage"
export CSF_STEPDefaults="$runtime/occt/XSTEPResource"
export CSF_IGESDefaults="$runtime/occt/XSTEPResource"
export CSF_PluginDefaults="$runtime/occt/StdResource"
export CSF_StandardDefaults="$runtime/occt/StdResource"
export CSF_XCAFDefaults="$runtime/occt/StdResource"
export CSF_MDTVTexturesDirectory="$runtime/occt/Textures"
export CSF_XmlOcafResource="$runtime/occt/XmlOcafResource"
exec "$runtime/bin/ZIMA-CAD-Parts" "$@"
