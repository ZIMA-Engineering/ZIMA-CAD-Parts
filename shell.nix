{ pkgs ? (import <nixpkgs> {}) }:
let
  f =
    { stdenv, qt5, qtcreator, pkgconfig, qmake, xlibsWrapper, openssl, gdb, gcc }:
    let
      zcpQt = qt5.env "qt-zcp-${qt5.qtbase.version}" (with qt5; [
        qtbase qtdeclarative qtdoc qtimageformats qtlocation qtquickcontrols qtquickcontrols2
        qtsvg qttools qttranslations qtwebengine qtwebchannel qtx11extras
      ]);
    in stdenv.mkDerivation {
      name = "ZIMA-CAD-Parts";
      src = ./.;
      nativeBuildInputs = [ zcpQt qtcreator pkgconfig qmake xlibsWrapper gdb ];
      buildInputs = [ openssl ];
      shellHook = ''
        mkdir -p nix-build
        ln -sfn ${zcpQt} nix-build/qt
        ln -sfn ${gdb} nix-build/gdb
        ln -sfn ${gcc} nix-build/gcc
      '';
    };
in pkgs.libsForQt5.callPackage f {}
