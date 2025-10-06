{
  description = "Event loop project with liburing";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
            liburing
            pkg-config
          ];

          shellHook = ''
            echo "Event loop development environment"
            echo "liburing version: $(pkg-config --modversion liburing 2>/dev/null || echo 'installed')"
          '';
        };
      });
}
