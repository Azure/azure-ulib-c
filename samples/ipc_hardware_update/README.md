# Sample ipc_hardware_update

## Simple producer Consumer sample emulating a display connect to the device using DCF interface.

This sample is composed by 3 different actors:

1. **Consumers**: The actors that will call functions in the display interface to clean the display and write messages. Those are the codes in the `consumers` directory.
2. **Producers**: The emulators that will expose the display interface. Those are the codes in the `producers` directory.
3. **OS**: This is the glue between producer and consumer, it create an instance of the IPC, install and uninstall the producers and consumers. To test the IPC, the OS calls  `my_consumer_do_display()` in the consumer that will try to use the display interface cleaning the screen and print a couple of messages.  

To demo a modular update, this sample contains 2 producers, *Contoso.200401* and *Contoso.480401*. Both expose the same interface display version 1.

At the beginning, the OS install Contoso.200401's emulator that publish the display v1 interface by calling `contoso_display_20x4_1_create()`. From that point, the interface display v1 is available in the IPC and any other component can start to use it.

The OS now install my_consumer by calling `my_consumer_create()`. My_consumer will use the display interface every time the OS calls `my_consumer_do_display()`.

```bash
My consumer try use display.1 interface...
My consumer got display.1 interface with success.
        +Contoso emulator----+
        |Hello world! This is|
        |      (\(\          |
        |     ( -.-)         |
        |     o_(")(")       |
        +--------------------+
My consumer try use display.1 interface...
        +Contoso emulator----+
        |Hello world! This is|
        |      /)/)          |
        |     ( -.-)         |
        |     o_(")(")       |
        +--------------------+
My consumer try use display.1 interface...
        +Contoso emulator----+
        |Hello world! This is|
        |      (\(\          |
        |     ( -.-)         |
        |     o_(")(")       |
        +--------------------+
```

To replace Contoso.200401's emulator by Contoso.480401's ones, the OS shall first remove Contoso.200401's code by calling `contoso_display_20x4_1_destroy()` and then install Contoso.480401's implementation by calling `contoso_display_48x4_1_create()`. During this process, if my_consumer try to use the interface in the `my_consumer_do_display()`, IPC will return `AZ_ERROR_ITEM_NOT_FOUND`, when my_consumer shall release the display handle.

```bash
My consumer try use display.1 interface...
display.1 was uninstalled.
Release the handle.
My consumer try use display.1 interface...
display.1 is not available.
```

Once Contoso.480401's emulator is installed, the `my_consumer_do_display()` shall result in a call to Contoso.480401's display.

```bash
My consumer try use display.1 interface...
My consumer got display.1 interface with success.
        +Contoso display emulator------------------------+
        |Hello world! This is a test to display a message|
        |      (\(\                                      |
        |     ( -.-)                                     |
        |     o_(")(")                                   |
        +------------------------------------------------+
My consumer try use display.1 interface...
        +Contoso display emulator------------------------+
        |Hello world! This is a test to display a message|
        |      /)/)                                      |
        |     ( -.-)                                     |
        |     o_(")(")                                   |
        +------------------------------------------------+
My consumer try use display.1 interface...
        +Contoso display emulator------------------------+
        |Hello world! This is a test to display a message|
        |      (\(\                                      |
        |     ( -.-)                                     |
        |     o_(")(")                                   |
        +------------------------------------------------+
```

If Contoso.480401's emulator is replaced again by Contoso.200401's one, and my_consumer try to use the handle from Contoso.480401, the IPC will return `AZ_ERROR_ITEM_NOT_FOUND`, when my_consumer shall release the display handle. After that, my_consumer can get the handle again, this time from Contoso.200401's emulator.

```bash
My consumer try use display.1 interface...
display.1 was uninstalled.
Release the handle.
My consumer try use display.1 interface...
My consumer got display.1 interface with success.
        +Contoso emulator----+
        |Hello world! This is|
        |      (\(\          |
        |     ( -.-)         |
        |     o_(")(")       |
        +--------------------+
My consumer try use display.1 interface...
        +Contoso emulator----+
        |Hello world! This is|
        |      /)/)          |
        |     ( -.-)         |
        |     o_(")(")       |
        +--------------------+
My consumer try use display.1 interface...
        +Contoso emulator----+
        |Hello world! This is|
        |      (\(\          |
        |     ( -.-)         |
        |     o_(")(")       |
        +--------------------+
```
