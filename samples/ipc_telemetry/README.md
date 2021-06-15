# Sample ipc_telemetry

## Simple producer Consumer sample using IPC telemetry.

This sample is composed by 3 different actors:

1. **Consumers**: The actor that will subscribe for telemetry in the sensors interface. These are
                    the codes in the `consumers` directory.
2. **Producers**: The actor that will expose the sensor interface with telemetry notification.
                    These are the codes in the `producers` directory.
3. **OS**: This is the glue between `producers` and `consumers`. It creates an instance of the IPC,
                    and installs the producer that provides telemetry for multiple sensors and
                    the consumer that subscribes to receive these telemetries.

### steps

1. At the beginning, the OS installs `sensors_v1i1` package that publishes the sensors.1 interface by
    calling `sensors_v1i1_create()`. From this point the interface sensors.1 is available in the IPC
    and any other module can subscribe to receive those telemetries.

2. The OS now installs my_consumer by calling `my_consumer_create()`. From this point, my_consumer
    will subscribe to the available telemetries and print these telemetries on the screen.
