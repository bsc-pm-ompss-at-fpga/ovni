{
  inputs.nixpkgs.url = "nixpkgs";
  inputs.bscpkgs.url = "git+https://git.sr.ht/~rodarima/bscpkgs";
  inputs.bscpkgs.inputs.nixpkgs.follows = "nixpkgs";

  nixConfig.bash-prompt = "\[nix-develop\]$ ";

  outputs = { self, nixpkgs, bscpkgs }:
  let
    pkgs = bscpkgs.outputs.legacyPackages.x86_64-linux;
    compilerList = with pkgs; [
      #gcc49Stdenv # broken
      gcc6Stdenv
      gcc7Stdenv
      gcc8Stdenv
      gcc9Stdenv
      gcc10Stdenv
      gcc11Stdenv
      gcc12Stdenv
      gcc13Stdenv
    ];
    lib = pkgs.lib;
  in {
    packages.x86_64-linux.ovniPackages = rec {
      # Build with the current source
      local = pkgs.ovni.overrideAttrs (old: {
        pname = "ovni-local";
        src = self;
      });

      # Build in Debug mode
      debug = local.overrideAttrs (old: {
        pname = "ovni-debug";
        cmakeBuildType = "Debug";
      });

      # Without MPI
      nompi = local.overrideAttrs (old: {
        pname = "ovni-nompi";
        buildInputs = lib.filter (x: x != pkgs.mpi ) old.buildInputs;
        cmakeFlags = old.cmakeFlags ++ [ "-DUSE_MPI=OFF" ];
      });

      # Helper function to build with different compilers
      genOvniCC = stdenv: (local.override {
        stdenv = stdenv;
      }).overrideAttrs (old: {
        name = "ovni-gcc" + stdenv.cc.cc.version;
        cmakeFlags = old.cmakeFlags ++ [
          "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF"
        ];
      });

      # Test several gcc versions
      compilers = let
        drvs = map genOvniCC compilerList;
      in pkgs.runCommand "ovni-compilers" { } ''
        printf "%s\n" ${builtins.toString drvs} > $out
      '';

      # Enable RT tests
      rt = (local.override {
        # Provide the clang compiler as default
        stdenv = pkgs.stdenvClangOmpss2;
      }).overrideAttrs (old: {
        pname = "ovni-rt";
        # We need to be able to exit the chroot to run Nanos6 tests, as they
        # require access to /sys for hwloc
        __noChroot = true;
        buildInputs = old.buildInputs ++ (with pkgs; [ pkg-config nosv nanos6 nodes ]);
        cmakeFlags = old.cmakeFlags ++ [ "-DENABLE_ALL_TESTS=ON" ];
        preConfigure = old.preConfigure or "" + ''
          export NODES_HOME="${pkgs.nodes}"
          export NANOS6_HOME="${pkgs.nanos6}"
        '';
      });

      # Build with ASAN and pass RT tests
      asan = rt.overrideAttrs (old: {
        pname = "ovni-asan";
        cmakeFlags = old.cmakeFlags ++ [ "-DCMAKE_BUILD_TYPE=Asan" ];
        # Ignore leaks in tests for now, only check memory errors
        preCheck = old.preCheck + ''
          export ASAN_OPTIONS=detect_leaks=0
        '';
      });
    };
  };
}
