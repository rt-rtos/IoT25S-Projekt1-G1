# PlatformIO extra script: make compile_commands.json carry the
# cross-toolchain's system include paths so clangd resolves <Arduino.h>
# dependencies against arm-none-eabi headers instead of the host's.
Import("env")
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)
