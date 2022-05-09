# Import("env")
#
# # General options that are passed to the C and C++ compilers
# env.Append(CCFLAGS=[
#     "-fno-common",
#     "-Wall",
#     "-Os",
#     "-g3",
#     "-D__VFP_FP__",
#     "-mcpu=cortex-m4",
#     "-mthumb",
#     "-mfpu=fpv4-sp-d16",
#     "-mfloat-abi=softfp",
#     "-ffunction-sections",
#     "-fdata-sections",
#     "-Wl,--gc-sections"
# ])
