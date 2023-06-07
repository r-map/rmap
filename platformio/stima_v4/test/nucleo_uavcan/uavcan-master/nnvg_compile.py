#Import("env")

import subprocess

print("execute","nnvg  --target-language c -O include   --target-endianness little --generate-support only") 
subprocess.check_output(["nnvg", "--target-language", "c", "-O", "include", "--target-endianness", "little", "--generate-support", "only"])

print("execute","nnvg -I data_types/uavcan -I data_types/rmap --allow-unregulated-fixed-port-id --target-language c -O include data_types/unreg --target-endianness little --generate-support never") 
subprocess.check_output(["nnvg","-I","data_types/uavcan", "-I", "data_types/uavcan", "--target-language", "c", "-O", "include","data_types/uavcan", "--target-endianness", "little", "--generate-support", "never"])

print("execute","nnvg -I data_types/uavcan -I data_types/rmap --allow-unregulated-fixed-port-id --target-language c -O include data_types/unreg --target-endianness little --generate-support never") 
subprocess.check_output(["nnvg","-I","data_types/uavcan", "-I", "data_types/rmap", "--allow-unregulated-fixed-port-id", "--target-language", "c", "-O", "include","data_types/rmap", "--target-endianness", "little", "--generate-support", "never"])