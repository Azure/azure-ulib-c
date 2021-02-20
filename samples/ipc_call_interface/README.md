# Sample ipc_call_interface

## Simple producer Consumer sample using IPC with producer update.

This sample is composed by 3 different actors:

1. **Consumer**: The actor that will call functions in the math interface. Those are the codes in the `consumer` directory.
2. **Producer**: The actor that will expose the math interface. Those are the codes in the `producer` directory.
3. **OS**: This is the glue between `producer` and `consumer`, it create an instance of the IPC, install and uninstall the producers and consumers, and it runs the `do_math` in the consumer that will try to use the math interface published by the producers.  

To demo the update, this sample contains 2 producers, *Contoso* and *Fabrikan*. Both expose the same interface math version 1.

At the beginning, the OS install Contoso's implementation that publish the math v1 interface by calling `contoso_math_1_create()`. From that point, the interface math v1 is available in the IPC and any other module can start to use that.

The OS now install my_consumer by calling `my_consumer_create()` that will try to use the math interface. As a result, it will invoke `sum` and `subtract` capabilities in Contoso's implementation. From this point, my_consumer will keep using this interface every time the OS calls `my_consumer_do_math()`.

To replace Contoso's implementation by Fabrikan's ones, the OS shall first remove Contoso's code by calling `contoso_math_1_destroy()` and after that install Frabikan's implementation by calling `fabrikan_math_1_create()`. During this process, if my_consumer try to use the interface in the `my_consumer_do_math()`, IPC will return `AZ_ERROR_ITEM_NOT_FOUND`, when my_consumer shall release the math handle.