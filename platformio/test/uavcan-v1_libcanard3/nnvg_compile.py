Import("env")
import os

if not os.path.isdir("include"):


    # install yakut in platformio virtualenv and use generate_types shortcut
    try:
        from nunavut import generate_types
    except ImportError:
        env.Execute(
            env.VerboseAction(
                '$PYTHONEXE -m pip install "yakut"',
                "Installing ESP-IDF's Python dependencies",
            )
        )

    from nunavut import generate_types
    generate_types("c", "data_types/uavcan", "include", omit_serialization_support=False)
    generate_types("c", "data_types/reg", "include", omit_serialization_support=False, lookup_directories=["data_types/uavcan",])



#  use subprocess to use system installed command nnvg

#    import subprocess
#
#    print("execute","nnvg  --target-language c -O include   --target-endianness little --generate-support only") 
#    subprocess.check_output(["nnvg", "--target-language", "c", "-O", "include", "--target-endianness", "little", "--generate-support", "only"])
#
#    print("execute","nnvg -I data_types/uavcan -I data_types/reg --target-language c -O include  data_types/reg --target-endianness little --generate-support never") 
#    subprocess.check_output(["nnvg","-I","data_types/uavcan", "-I", "data_types/reg", "--target-language", "c", "-O", "include","data_types/reg", "--target-endianness", "little", "--generate-support", "never"])
#
#    print("execute","nnvg -I data_types/uavcan -I data_types/reg --target-language c -O include  data_types/uavcan --target-endianness little --generate-support never") 
#    subprocess.check_output(["nnvg","-I","data_types/uavcan", "-I", "data_types/reg", "--target-language", "c", "-O", "include","data_types/uavcan", "--target-endianness", "little", "--generate-support", "never"])




# try to use system installed nunavut and pydsdl python modules

#from pydsdl import read_namespace
#from nunavut import build_namespace_tree
#from nunavut.lang import LanguageContext
#from nunavut.jinja import DSDLCodeGenerator
#from pathlib import Path


#include_paths = ["data_types/uavcan","data_types/reg"]
#root_namespace_dir = gen_paths.root_dir / Path("submodules") / Path("public_regulated_data_types") / Path("uavcan")
#compound_types
#root_ns_folder
#out_dir



## parse the dsdl
#compound_types = read_namespace(root_namespace, include_paths)

## select a target language
#language_context = LanguageContext('c')

## build the namespace tree
#root_namespace = build_namespace_tree(compound_types,
#                                      root_ns_folder,
#                                      out_dir,
#                                      language_context)

## give the root namespace to the generator and...
#generator = DSDLCodeGenerator(root_namespace)

## generate all the code!
#generator.generate_all()
