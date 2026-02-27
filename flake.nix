{
  description = "A Simple Skeleton for a Logos Storage App";

  inputs = {
    # Follow the same nixpkgs as logos-liblogos to ensure compatibility
    nixpkgs.follows = "logos-liblogos/nixpkgs";
    logos-cpp-sdk.url = "github:logos-co/logos-cpp-sdk?ref=feat/logos-result";
    logos-liblogos.url = "github:logos-co/logos-liblogos?ref=fix/logos-cleanup-on-terminate";
    logos-storage-module.url = "github:logos-co/logos-storage-module";
    logos-capability-module.url = "github:logos-co/logos-capability-module";
    logos-liblogos.inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
    logos-storage-module.inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
    logos-capability-module.inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
  };

  outputs = { self, nixpkgs, logos-cpp-sdk, logos-liblogos, logos-storage-module, logos-capability-module }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "aarch64-linux" "x86_64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
        logosSdk = logos-cpp-sdk.packages.${system}.default;
        logosLiblogos = logos-liblogos.packages.${system}.default;
        logosStorageModule = logos-storage-module.packages.${system}.default;
        logosCapabilityModule = logos-capability-module.packages.${system}.default;
      });
    in
    {
      packages = forAllSystems ({ pkgs, logosSdk, logosLiblogos, logosStorageModule, logosCapabilityModule }:
        let
          # Common configuration
          common = import ./nix/default.nix {
            inherit pkgs logosSdk logosLiblogos logosStorageModule;
          };
          src = ./.;

          # Library package
          lib = import ./nix/lib.nix {
            inherit pkgs common src logosStorageModule logosSdk;
          };

          # CLI package
          app = import ./nix/app.nix {
            inherit pkgs common src logosLiblogos logosSdk logosStorageModule logosCapabilityModule;
          };

        in
        {
          # Individual outputs
          lib = lib;
          app = app;

          # Default package
          default = app;
        }
      );

      devShells = forAllSystems ({ pkgs, logosSdk, logosLiblogos, logosStorageModule, logosCapabilityModule }: {
        default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
            pkgs.pkg-config
          ];
          buildInputs = [
            pkgs.qt6.qtbase
            pkgs.qt6.qtremoteobjects
            pkgs.zstd
            pkgs.krb5
            pkgs.abseil-cpp
          ];

          shellHook = ''
            export LOGOS_CPP_SDK_ROOT="${logosSdk}"
            export LOGOS_LIBLOGOS_ROOT="${logosLiblogos}"
            export LOGOS_STORAGE_ROOT="${logosStorageModule}"
            echo "Logos Storage UI development environment"
            echo "LOGOS_CPP_SDK_ROOT: $LOGOS_CPP_SDK_ROOT"
            echo "LOGOS_LIBLOGOS_ROOT: $LOGOS_LIBLOGOS_ROOT"
            echo "LOGOS_STORAGE_ROOT: $LOGOS_STORAGE_ROOT"
          '';
        };
      });
    };
}
