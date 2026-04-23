from os.path import join
Import("env", "projenv")

# Dump global construction environment (for debug purpose)
#print(env.Dump())

# Custom BIN from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(" ".join(
        [
        "mkdir", "-p", "$PROJECT_DIR/bin/${PIOENV};",
        "cp",
        "$BUILD_DIR/${PROGNAME}.bin",
        "$PROJECT_DIR/bin/${PIOENV}/firmware.bin;"
        "cp",
        "$BUILD_DIR/bootloader.bin",
        "$PROJECT_DIR/bin/${PIOENV}/;"
        "cp",
        "$BUILD_DIR/partitions.bin",
        "$PROJECT_DIR/bin/${PIOENV}/;"
        "cp",
        "$PROJECT_PACKAGES_DIR/framework-arduinoespressif32/tools/partitions/boot_app0.bin",
        "$PROJECT_DIR/bin/${PIOENV}/"
        ]
    ), "Building $TARGET"))
