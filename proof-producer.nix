{ lib,
  stdenv,
  ninja,
  pkg-config,
  cmake,
  boost,
  gdb,
  lldb,
  cmake_modules,
  enableDebugging,
  gtest,
  enableDebug ? false,
  runTests ? false,
  }:
let
  inherit (lib) optional;
in stdenv.mkDerivation {
  name = "Proof-producer";

  src = lib.sourceByRegex ./. ["^proof-producer(/.*)?$" "^crypto3(/.*)?$" "^parallel-crypto3(/.*)?$" "CMakeLists.txt"];
  hardeningDisable = [ "fortify" ];

  nativeBuildInputs = [ cmake ninja pkg-config ] ++
                       (lib.optional (!stdenv.isDarwin) gdb) ++
                       (lib.optional (stdenv.isDarwin) lldb);

  # enableDebugging will keep debug symbols in boost
  propagatedBuildInputs = [ (if enableDebug then (enableDebugging boost) else boost) ];

  buildInputs = [cmake_modules gtest];

  cmakeFlags =
    [
      "-DCMAKE_INSTALL_PREFIX=${placeholder "out"}"
      (if enableDebug then "-DCMAKE_BUILD_TYPE=Debug" else "-DCMAKE_BUILD_TYPE=Release")
      (if runTests then "-DENABLE_TESTS=ON" else "-DENABLE_TESTS=OFF")
      "-DPROOF_PRODUCER_ENABLE=TRUE"
      "-G Ninja"
    ];

  doCheck = runTests;

  checkPhase = ''
    cd proof-producer && ctest --verbose --output-on-failure -R && cd ..
  '';

  shellHook = ''
    PS1="\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ "
    echo "Welcome to Proof-producer development environment!"
  '';
}
