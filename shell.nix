{ pkgs ? (import <nixpkgs> {}) }:
let
  f =
    { stdenv, qt5Full, qtcreator, pkgconfig, qmake, x11, openssl, gdb }:
    stdenv.mkDerivation {
      name = "ZIMA-CAD-Parts";
      src = ./.;
      nativeBuildInputs = [ qt5Full qtcreator pkgconfig qmake x11 gdb ];
      buildInputs = [ openssl ];
      shellHook = ''
        echo ${qt5Full}
      '';
    };
in pkgs.libsForQt5.callPackage f {}
