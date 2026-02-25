# Common build configuration shared across all packages
{ pkgs, logosSdk, logosLiblogos, logosStorageModule }:

{
  pname = "logos-storage-ui";
  version = "1.0.0";
  
  # Common native build inputs
  nativeBuildInputs = [ 
    pkgs.cmake 
    pkgs.ninja 
    pkgs.pkg-config
    pkgs.qt6.wrapQtAppsHook
  ];
  
  # Common runtime dependencies
  buildInputs = [
    pkgs.qt6.qtbase
    pkgs.qt6.qtremoteobjects
    pkgs.zstd
    pkgs.krb5
    pkgs.abseil-cpp
  ];
  
  # Common CMake flags
  cmakeFlags = [ 
    "-GNinja"
    "-DLOGOS_CPP_SDK_ROOT=${logosSdk}"
    "-DLOGOS_LIBLOGOS_ROOT=${logosLiblogos}"
    "-DLOGOS_STORAGE_ROOT=${logosStorageModule}"
  ];
  
  # Environment variables
  env = {
    LOGOS_CPP_SDK_ROOT = "${logosSdk}";
    LOGOS_LIBLOGOS_ROOT = "${logosLiblogos}";
    LOGOS_STORAGE_ROOT = "${logosStorageModule}";
  };
  
  # Metadata
  meta = with pkgs.lib; {
    description = "A Simple Skeleton for a Logos Storage App";
    platforms = platforms.unix;
  };
}
