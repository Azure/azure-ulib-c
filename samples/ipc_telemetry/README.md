# Sample ipc_telemetry

## Simple producer Consumer sample using IPC telemetry.

This sample is composed by 3 different actors:

1. **Consumers**: The actor that will subscribe for telemetry in the sensors interface. Those are
                    the codes in the `consumers` directory.
2. **Producers**: The actor that will expose the sensor interface with telemetry notification.
                    Those are the codes in the `producers` directory.
3. **OS**: This is the glue between `producers` and `consumers`, it create an instance of the IPC,
                    install the producer that provides telemetry for multiple sensors and a
                    consumer that subscribe to receive those telemetries.

### steps

1. At the beginning, the OS install `sensors_v1i1` that publish the sensors.1 interface by calling
    `sensors_v1i1_create()`. From that point, the interface sensors.1 is available in the IPC and
    any other module can subscribe to receive those telemetries.

2. The OS now install my_consumer by calling `my_consumer_create()`. From this point, my_consumer
    will subscribe for the available telemetries and print those telemetries in the screen. 
