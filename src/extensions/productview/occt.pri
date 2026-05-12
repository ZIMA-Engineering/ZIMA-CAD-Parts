OCCT_AVAILABLE = 0

isEmpty(OCCT_ROOT): OCCT_ROOT = $$(OCCT_ROOT)

!isEmpty(OCCT_ROOT) {
    exists($$OCCT_ROOT/include/opencascade) {
        OCCT_INCDIR = $$OCCT_ROOT/include/opencascade
    } else:exists($$OCCT_ROOT/include) {
        OCCT_INCDIR = $$OCCT_ROOT/include
    } else:exists($$OCCT_ROOT/inc) {
        OCCT_INCDIR = $$OCCT_ROOT/inc
    }

    exists($$OCCT_ROOT/lib) {
        OCCT_LIBDIR = $$OCCT_ROOT/lib
    }

    exists($$OCCT_ROOT/bin) {
        OCCT_BINDIR = $$OCCT_ROOT/bin
    }

    !isEmpty(OCCT_INCDIR):!isEmpty(OCCT_LIBDIR):exists($$OCCT_INCDIR):exists($$OCCT_LIBDIR) {
        OCCT_AVAILABLE = 1
        DEFINES += HAVE_OCCT
        INCLUDEPATH += $$OCCT_INCDIR
        LIBS += -L$$OCCT_LIBDIR
        unix:QMAKE_RPATHDIR += $$OCCT_LIBDIR
        win32:!isEmpty(OCCT_BINDIR):LIBS += -L$$OCCT_BINDIR
        win32:LIBS += -lopengl32 -luser32

        OCCT_COMMON_LIBS = \
            -lTKernel -lTKMath -lTKG2d -lTKG3d -lTKGeomBase -lTKGeomAlgo \
            -lTKBRep -lTKTopAlgo -lTKPrim -lTKShHealing -lTKMesh \
            -lTKService -lTKV3d -lTKOpenGl \
            -lTKCDF -lTKLCAF -lTKCAF -lTKBinL -lTKXmlL -lTKBin -lTKXml \
            -lTKStdL -lTKStd -lTKVCAF -lTKXCAF -lTKXSBase

        OCCT_HAS_TKDESTEP = 0
        exists($$OCCT_LIBDIR/libTKDESTEP.dylib): OCCT_HAS_TKDESTEP = 1
        exists($$OCCT_LIBDIR/libTKDESTEP.so): OCCT_HAS_TKDESTEP = 1
        exists($$OCCT_LIBDIR/TKDESTEP.lib): OCCT_HAS_TKDESTEP = 1

        equals(OCCT_HAS_TKDESTEP, 1) {
            OCCT_STEP_LIBS = -lTKDESTEP
        } else {
            OCCT_STEP_LIBS = \
                -lTKSTEPBase -lTKSTEPAttr -lTKSTEP209 \
                -lTKSTEP -lTKXDESTEP
        }

        OCCT_HAS_TKDEIGES = 0
        exists($$OCCT_LIBDIR/libTKDEIGES.dylib): OCCT_HAS_TKDEIGES = 1
        exists($$OCCT_LIBDIR/libTKDEIGES.so): OCCT_HAS_TKDEIGES = 1
        exists($$OCCT_LIBDIR/TKDEIGES.lib): OCCT_HAS_TKDEIGES = 1

        equals(OCCT_HAS_TKDEIGES, 1) {
            OCCT_IGES_LIBS = -lTKDEIGES
        } else {
            OCCT_IGES_LIBS = -lTKIGES -lTKXDEIGES
        }

        OCCT_HAS_TKDESTL = 0
        exists($$OCCT_LIBDIR/libTKDESTL.dylib): OCCT_HAS_TKDESTL = 1
        exists($$OCCT_LIBDIR/libTKDESTL.so): OCCT_HAS_TKDESTL = 1
        exists($$OCCT_LIBDIR/TKDESTL.lib): OCCT_HAS_TKDESTL = 1

        OCCT_HAS_TKRWMESH = 0
        exists($$OCCT_LIBDIR/libTKRWMesh.dylib): OCCT_HAS_TKRWMESH = 1
        exists($$OCCT_LIBDIR/libTKRWMesh.so): OCCT_HAS_TKRWMESH = 1
        exists($$OCCT_LIBDIR/TKRWMesh.lib): OCCT_HAS_TKRWMESH = 1

        equals(OCCT_HAS_TKDESTL, 1):equals(OCCT_HAS_TKRWMESH, 1) {
            OCCT_STL_LIBS = -lTKDESTL -lTKRWMesh
        } else {
            OCCT_STL_LIBS = -lTKSTL
        }

        LIBS += \
            $$OCCT_COMMON_LIBS \
            $$OCCT_STEP_LIBS \
            $$OCCT_IGES_LIBS \
            $$OCCT_STL_LIBS
        message("OCCT found in $$OCCT_ROOT; STEP/IGES/STL preview enabled")
    } else {
        warning("OCCT not found; STEP/IGES/STL preview disabled")
    }
}
