# Import("env")
#
# # General options that are passed to the C and C++ compilers
# env.Append(CCFLAGS=[
#     "-fno-common",
#     "-Wall",
#     "-Os",
#     "-g3",
#     "-mcpu=cortex-m4",
#     "-mthumb",
#     "-mfpu=fpv4-sp-d16",
#     "-mfloat-abi=softfp",
#     "-ffunction-sections",
#     "-fdata-sections",
#     "-Wl,--gc-sections",
#     "-D_WINSOCK_H",
#     "-D__error_t_defined",
#     "-DHAL_RNG_MODULE_ENABLED",
#     "-fexceptions",
#     "-Iinclude"
# ])
#
# # General options that are passed to the C++ compilers
# env.Append(CXXFLAGS=["-std=c++11"])
#
# # Import("env")
# #
# # # General options that are passed to the C and C++ compilers
# # env.Append(CCFLAGS=[
# #     "-fno-common",
# #     "-Wall",
# #     "-Os",
# #     "-g3",
# #     "-D__VFP_FP__",
# #     "-mcpu=cortex-m4",
# #     "-mthumb",
# #     "-mfpu=fpv4-sp-d16",
# #     "-mfloat-abi=softfp",
# #     "-ffunction-sections",
# #     "-fdata-sections",
# #     "-Wl,--gc-sections"
# # ])
