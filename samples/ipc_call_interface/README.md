# Sample ipc_call_interface

## Simple producer Consumer sample using IPC with producer update.

This sample is composed by 3 different actors:

1. **Consumers**: The actor that will call functions in the cipher interface. Those are the codes in the `consumers` directory.
2. **Producers**: The actor that will expose the cipher interface. Those are the codes in the `producers` directory.
3. **OS**: This is the glue between `producers` and `consumers`, it create an instance of the IPC, install and uninstall the producers and consumers, and it runs the `my_consumer_do_cipher()` that will try to use the cipher interface to encrypt and decrypt a text.

To demonstrate a modular update, this sample contains 2 versions of the cipher producer. Both expose the same interface cipher version 1, however, `cipher_v1i1` only accept context `0` and `cipher_v2i1` accepts contexts `0` and `1`. Context is just a way to differentiate the cipher key.

### steps

1. At the beginning, the OS install `cipher_v1i1` that publish the cipher v1 interface by calling `cipher_v1i1_create()`. From that point, the interface cipher v1 is available in the IPC and any other module can start to use that.

2. The OS now install my_consumer by calling `my_consumer_create()`. From this point, my_consumer will keep using the cipher interface every time the OS calls `my_consumer_do_cipher()`. The OS will call it for context `0` and `1`. Because `cipher_v1i1` only accept context `0`, my consumer call to context `1` will fail.

3. To replace `cipher_v1i1` by `cipher_v2i1`, the OS shall first remove `cipher_v1i1` by calling `cipher_v1i1_destroy()` and after that install `cipher_v2i1` by calling `cipher_v2i1_create()`. During this process, if my_consumer try to use the interface in the `my_consumer_do_cipher()`, IPC will return `AZ_ERROR_ITEM_NOT_FOUND`.

4. Once `cipher_v1i1` is installed, `my_consumer_do_cipher()` will succeed on both contexts `0` and `1`.
