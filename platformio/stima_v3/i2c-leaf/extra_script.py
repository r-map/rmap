from os.path import join
Import("env", "projenv")

# Dump global construction environment (for debug purpose)
#print(env.Dump())

# Custom BIN from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "mkdir", "-p", "$PROJECT_DIR/bin/${PIOENV};",
        "$OBJCOPY",
        "-O",
        "binary",
        "$TARGET",
        "$PROJECT_DIR/bin/${PIOENV}/FIRMWARE.BIN"
    ]), "Building $TARGET"))
