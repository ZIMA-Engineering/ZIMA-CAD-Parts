{ pkgs ? (import <nixpkgs> {}) }:
let
  f =
    { stdenv, qt6, qtcreator, pkg-config, qmake, openssl, gdb, gcc }:
    let
      zcpQt = qt6.env "qt-zcp-${qt6.qtbase.version}" (with qt6; [
        qtbase qtdeclarative qtimageformats qtlocation qtsvg qttools
        qttranslations qtwebengine qtwebchannel qtpositioning
      ]);
    in stdenv.mkDerivation {
      name = "ZIMA-CAD-Parts";
      src = ./.;
      nativeBuildInputs = [ zcpQt qtcreator pkg-config qmake gdb ];
      buildInputs = [ openssl ];
      shellHook = ''
        mkdir -p nix-build
        ln -sfn ${zcpQt} nix-build/qt
        ln -sfn ${gdb} nix-build/gdb
        ln -sfn ${gcc} nix-build/gcc
      '';
    };
in pkgs.qt6Packages.callPackage f {}
