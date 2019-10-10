# Azure uStream - Samples

## uStream Basic

This sample will create two ustreams: the first manages dynamic memory allocated in the heap and the second manages static memory (constant memory). The second ustream is then concatenated to the first and the resulting ustream is fully printed to the console. Further description of this sample can be found [here](https://github.com/Azure/azure-ulib-c/blob/8c0aacd685748ae610ad9ede1aba94f3b6c43cc3/samples/ustream_basic/src/main.c#L43) in the `main.c` file.

## uStream Split

This sample demonstrates the ability to split a ustream at a desired position into two ustreams.
The result of this operation gives the first ustream truncated at the position passed (not inclusive)
to `az_ustream_split()` and the second ustream begins at the input position and ends at the end of
the original input ustream.
