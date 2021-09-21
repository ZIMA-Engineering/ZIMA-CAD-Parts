{ pkgs ? (import <nixpkgs> {}) }:
let
  f =
    { stdenv, qt5Full, qtcreator, pkgconfig, qmake, x11, openssl, gdb, gcc }:
    stdenv.mkDerivation {
      name = "ZIMA-CAD-Parts";
      src = ./.;
      nativeBuildInputs = [ qt5Full qtcreator pkgconfig qmake x11 gdb ];
      buildInputs = [ openssl ];
      shellHook = ''
        mkdir -p nix-build
        ln -sfn ${qt5Full} nix-build/qt
        ln -sfn ${gdb} nix-build/gdb
        ln -sfn ${gcc} nix-build/gcc
      '';
    };
in pkgs.libsForQt5.callPackage f {}
