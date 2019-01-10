{ pkgs ? (import <nixpkgs> {}) }:
let
  f =
    { stdenv, qt59, qtcreator, pkgconfig, qmake, x11, openssl, gdb }:
    stdenv.mkDerivation {
      name = "ZIMA-CAD-Parts";
      src = ./.;
      nativeBuildInputs = [ qt59.full qtcreator pkgconfig qmake x11 gdb ];
      buildInputs = [ openssl ];
      shellHook = ''
        echo ${qt59.full}
      '';
    };
in pkgs.libsForQt59.callPackage f {}
