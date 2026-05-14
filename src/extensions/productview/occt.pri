OCCT_AVAILABLE = 0

defineTest(occtHasLibrary) {
    OCCT_CHECK_LIB = $$1

    exists($$OCCT_LIBDIR/lib$${OCCT_CHECK_LIB}.dylib): return(true)
    exists($$OCCT_LIBDIR/lib$${OCCT_CHECK_LIB}.so): return(true)
    exists($$OCCT_LIBDIR/$${OCCT_CHECK_LIB}.lib): return(true)

    return(false)
}

isEmpty(OCCT_ROOT): OCCT_ROOT = $$(OCCT_ROOT)

OCCT_PREFIXES =
OCCT_MULTIARCH =
unix:!macx {
    OCCT_MULTIARCH = $$system(gcc -print-multiarch 2>/dev/null)
}

!isEmpty(OCCT_ROOT) {
    OCCT_PREFIXES += $$OCCT_ROOT
} else:unix:!macx {
    OCCT_PREFIXES += /usr /usr/local
}

for(OCCT_PREFIX, OCCT_PREFIXES) {
    equals(OCCT_AVAILABLE, 0) {
        OCCT_INCDIRS =
        exists($$OCCT_PREFIX/include/opencascade) {
            OCCT_INCDIRS += $$OCCT_PREFIX/include/opencascade
        }
        exists($$OCCT_PREFIX/include) {
            OCCT_INCDIRS += $$OCCT_PREFIX/include
        }
        exists($$OCCT_PREFIX/inc) {
            OCCT_INCDIRS += $$OCCT_PREFIX/inc
        }

        OCCT_LIBDIRS =
        !isEmpty(OCCT_MULTIARCH):exists($$OCCT_PREFIX/lib/$$OCCT_MULTIARCH) {
            OCCT_LIBDIRS += $$OCCT_PREFIX/lib/$$OCCT_MULTIARCH
        }
        exists($$OCCT_PREFIX/lib64) {
            OCCT_LIBDIRS += $$OCCT_PREFIX/lib64
        }
        exists($$OCCT_PREFIX/lib) {
            OCCT_LIBDIRS += $$OCCT_PREFIX/lib
        }

        OCCT_BINDIR =
        exists($$OCCT_PREFIX/bin) {
            OCCT_BINDIR = $$OCCT_PREFIX/bin
        }

        for(OCCT_INCDIR_CANDIDATE, OCCT_INCDIRS) {
            equals(OCCT_AVAILABLE, 0) {
                for(OCCT_LIBDIR_CANDIDATE, OCCT_LIBDIRS) {
                    equals(OCCT_AVAILABLE, 0) {
                        OCCT_INCDIR = $$OCCT_INCDIR_CANDIDATE
                        OCCT_LIBDIR = $$OCCT_LIBDIR_CANDIDATE

                        OCCT_COMMON_LIB_NAMES = \
                            TKernel TKMath TKG2d TKG3d TKGeomBase TKGeomAlgo \
                            TKBRep TKTopAlgo TKPrim TKShHealing TKMesh \
                            TKService TKV3d TKOpenGl \
                            TKCDF TKLCAF TKCAF TKBinL TKXmlL TKBin TKXml \
                            TKStdL TKStd TKVCAF TKXCAF TKXSBase

                        OCCT_COMMON_LIBS =
                        OCCT_MISSING_LIBS =
                        for(OCCT_LIB_NAME, OCCT_COMMON_LIB_NAMES) {
                            occtHasLibrary($$OCCT_LIB_NAME) {
                                OCCT_COMMON_LIBS += -l$${OCCT_LIB_NAME}
                            } else {
                                OCCT_MISSING_LIBS += $$OCCT_LIB_NAME
                            }
                        }

                        OCCT_STEP_LIBS =
                        occtHasLibrary(TKDESTEP) {
                            OCCT_STEP_LIBS = -lTKDESTEP
                        } else {
                            OCCT_STEP_LIB_NAMES = \
                                TKSTEPBase TKSTEPAttr TKSTEP209 TKSTEP TKXDESTEP
                            OCCT_STEP_MISSING_LIBS =
                            for(OCCT_LIB_NAME, OCCT_STEP_LIB_NAMES) {
                                occtHasLibrary($$OCCT_LIB_NAME) {
                                    OCCT_STEP_LIBS += -l$${OCCT_LIB_NAME}
                                } else {
                                    OCCT_STEP_MISSING_LIBS += $$OCCT_LIB_NAME
                                }
                            }
                            !isEmpty(OCCT_STEP_MISSING_LIBS): OCCT_STEP_LIBS =
                        }

                        OCCT_IGES_LIBS =
                        occtHasLibrary(TKDEIGES) {
                            OCCT_IGES_LIBS = -lTKDEIGES
                        } else {
                            OCCT_IGES_LIB_NAMES = TKIGES TKXDEIGES
                            OCCT_IGES_MISSING_LIBS =
                            for(OCCT_LIB_NAME, OCCT_IGES_LIB_NAMES) {
                                occtHasLibrary($$OCCT_LIB_NAME) {
                                    OCCT_IGES_LIBS += -l$${OCCT_LIB_NAME}
                                } else {
                                    OCCT_IGES_MISSING_LIBS += $$OCCT_LIB_NAME
                                }
                            }
                            !isEmpty(OCCT_IGES_MISSING_LIBS): OCCT_IGES_LIBS =
                        }

                        OCCT_STL_LIBS =
                        occtHasLibrary(TKDESTL):occtHasLibrary(TKRWMesh) {
                            OCCT_STL_LIBS = -lTKDESTL -lTKRWMesh
                        } else:occtHasLibrary(TKSTL) {
                            OCCT_STL_LIBS = -lTKSTL
                        }

                        isEmpty(OCCT_MISSING_LIBS):!isEmpty(OCCT_STEP_LIBS):!isEmpty(OCCT_IGES_LIBS):!isEmpty(OCCT_STL_LIBS) {
                            OCCT_AVAILABLE = 1
                            OCCT_FOUND_ROOT = $$OCCT_PREFIX
                            DEFINES += HAVE_OCCT
                            INCLUDEPATH += $$OCCT_INCDIR
                            LIBS += -L$$OCCT_LIBDIR

                            OCCT_ADD_RPATH = 1
                            unix {
                                equals(OCCT_LIBDIR, /usr/lib): OCCT_ADD_RPATH = 0
                                equals(OCCT_LIBDIR, /usr/lib64): OCCT_ADD_RPATH = 0
                                !isEmpty(OCCT_MULTIARCH):equals(OCCT_LIBDIR, /usr/lib/$$OCCT_MULTIARCH): OCCT_ADD_RPATH = 0
                            }
                            unix:equals(OCCT_ADD_RPATH, 1):QMAKE_RPATHDIR += $$OCCT_LIBDIR

                            win32:!isEmpty(OCCT_BINDIR):LIBS += -L$$OCCT_BINDIR
                            win32:LIBS += -lopengl32 -luser32

                            LIBS += \
                                $$OCCT_COMMON_LIBS \
                                $$OCCT_STEP_LIBS \
                                $$OCCT_IGES_LIBS \
                                $$OCCT_STL_LIBS

                            message("OCCT found in $$OCCT_FOUND_ROOT ($$OCCT_LIBDIR); STEP/IGES/STL preview enabled")
                        }
                    }
                }
            }
        }
    }
}

equals(OCCT_AVAILABLE, 0) {
    !isEmpty(OCCT_ROOT) {
        warning("OCCT not found in $$OCCT_ROOT; STEP/IGES/STL preview disabled")
    } else {
        warning("OCCT not found; STEP/IGES/STL preview disabled")
    }
}
