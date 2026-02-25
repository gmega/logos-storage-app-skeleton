{
  description = "Logos Storage UI - A Qt UI plugin for Logos Storage";

  inputs = {
    # Follow the same nixpkgs as logos-liblogos to ensure compatibility
    nixpkgs.follows = "logos-liblogos/nixpkgs";
   #    logos-cpp-sdk.url =  "/home/arnaud/Work/logos/logos-cpp-sdk";
    logos-cpp-sdk.url = "github:logos-co/logos-cpp-sdk?ref=feat/logos-result";
    logos-liblogos.url = "github:logos-co/logos-liblogos?ref=fix/logos-cleanup-on-terminate";
    logos-storage-module.url = "github:logos-co/logos-storage-module";
    #logos-storage-module.url = "path:/home/arnaud/Work/logos/logos-storage-module";
    logos-capability-module.url = "github:logos-co/logos-capability-module";
    logos-design-system.url = "github:logos-co/logos-design-system";
    logos-liblogos.inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
    logos-storage-module.inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
    logos-capability-module.inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
  };

  outputs = { self, nixpkgs, logos-cpp-sdk, logos-liblogos, logos-storage-module, logos-capability-module, logos-design-system }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "aarch64-linux" "x86_64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
        logosSdk = logos-cpp-sdk.packages.${system}.default;
        logosLiblogos = logos-liblogos.packages.${system}.default;
        logosStorageModule = logos-storage-module.packages.${system}.default;
        logosCapabilityModule = logos-capability-module.packages.${system}.default;
        logosDesignSystem = logos-design-system.packages.${system}.default;
      });
    in
    {
      packages = forAllSystems ({ pkgs, logosSdk, logosLiblogos, logosStorageModule, logosCapabilityModule, logosDesignSystem }:
        let
          # Common configuration
          common = import ./nix/default.nix { 
            inherit pkgs logosSdk logosLiblogos logosStorageModule logosDesignSystem;
          };
          src = ./.;

          # Library package
          lib = import ./nix/lib.nix { 
            inherit pkgs common src logosStorageModule logosSdk;
          };

          # CLI package
          cli = import ./nix/cli.nix {
            inherit pkgs common src logosLiblogos logosSdk logosStorageModule logosCapabilityModule;
          };

        in
        {
          # Individual outputs
          lib = lib;
          cli = cli;

          # Default package
          default = cli;
        }
      );

      devShells = forAllSystems ({ pkgs, logosSdk, logosLiblogos, logosStorageModule, logosCapabilityModule, logosDesignSystem }: {
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
            export LOGOS_DESIGN_SYSTEM_ROOT="${logosDesignSystem}"
            echo "Logos Storage UI development environment"
            echo "LOGOS_CPP_SDK_ROOT: $LOGOS_CPP_SDK_ROOT"
            echo "LOGOS_LIBLOGOS_ROOT: $LOGOS_LIBLOGOS_ROOT"
            echo "LOGOS_STORAGE_ROOT: $LOGOS_STORAGE_ROOT"
          '';
        };
      });
    };
}
