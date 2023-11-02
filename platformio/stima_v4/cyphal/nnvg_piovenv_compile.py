Import("env")
import os

if not os.path.isfile("../cyphal/include/done"):


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
    generate_types("c", "../cyphal/data_types/uavcan", "../cyphal/include", omit_serialization_support=False,language_options={"target_endianness":"little"})
    #generate_types("c", "../cyphal/data_types/reg", "../cyphal/include", omit_serialization_support=False, lookup_directories=["../cyphal/data_types/uavcan",],language_options={"target_endianness":"little"})
    generate_types("c", "../cyphal/data_types/rmap", "../cyphal/include", omit_serialization_support=False, lookup_directories=["../cyphal/data_types/uavcan",],language_options={"target_endianness":"little"})

    with open('../cyphal/include/done','w') as donefile:
        donefile.write("done")
