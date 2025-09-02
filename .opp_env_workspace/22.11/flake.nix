{
        inputs = {
            nixpkgs.url = "nixpkgs/22.11";
            flake-utils.url = "github:numtide/flake-utils";
        };
        outputs = { self, nixpkgs, flake-utils }:
        flake-utils.lib.eachDefaultSystem(system:
        let
            overlay = final: prev: {
                # allow python2 to be installed despite being EOL and having known vulnerabilities
                python2 = prev.python2.overrideAttrs (oldAttrs: {
                    meta = oldAttrs.meta // { knownVulnerabilities = []; };
                });
            };
            pkgs = import nixpkgs { inherit system; overlays = [ overlay ]; };
            in rec {
                devShells = rec {
                    default = pkgs.llvmPackages.stdenv.mkDerivation {
                        name = "run_command";
                        hardeningDisable = [ "all" ];
                        buildInputs = with pkgs; [ bashInteractive gitFull openssh curl gzip which gnused gnutar perl findutils coreutils bashInteractive ];
                        shellHook = ''
                            set -eo pipefail
                            cd /Users/rodrigo/omnetpp-workspace/fanet-simples/omnetpp-6.2.0 && curl -L --fail --progress-bar https://github.com/omnetpp/omnetpp/releases/download/omnetpp-6.2.0/omnetpp-6.2.0-macos-x86_64.tgz | tar --strip-components=1 -xzf - 2>/Users/rodrigo/omnetpp-workspace/fanet-simples/omnetpp-6.2.0/tar.log
                        '';
                    };
                };
            });
        }