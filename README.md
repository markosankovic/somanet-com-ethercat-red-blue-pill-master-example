SOMANET COM EtherCAT Red Blue Pill Master Example
=================================================

This is the EtherCAT master example program for [SOMANET CORE C22](https://doc.synapticon.com/hardware/core-c22/revision-a5/index.html) and [SOMANET COM EtherCAT](https://doc.synapticon.com/hardware/com-ethercat/revision-a4/index.html) stack from [Synapticon](https://www.synapticon.com/).

This program sends the PDO named "Offered pill" to slave and receives the "Pill taken" PDO. The value of "Offered pill" PDO can be changed by sending the appropriate signal to the running process.

How to run
----------

The slave EtherCAT module must have the right PDO mapping. There are two ways to ensure this:

1. Generate SII and write to EtherCAT EEPROM.

        $ siitool -o eeprom.bin esi.xml
        $ ethercat sii_write eeprom.bin
        $ sudo service ethercat restart # restart is required after eeprom write

2. Let the master program configure the PDO mapping by setting the value of [CONFIGURE_PDOS](https://github.com/markosankovic/somanet-com-ethercat-red-blue-pill-master-example/blob/master/somanet_com_ethercat_red_blue_pill_master_example.c#L16) define directive to 1.

Compile and run:

    $ make
    $ ./somanet_com_ethercat_red_blue_pill_master_example

The program will output:

    Pid: 15404
    Activating master...
    Starting timer...
    Started.
    Master is now sending red pill.
    Control master by sending signal to this process:
        kill -SIGUSR1 15404 # send red pill
        kill -SIGUSR2 15404 # send blue pill

Common errors
-------------

The master module outputs information about its state and events to the kernel ring buffer. These also end up in the system logs. You can get more information about EtherCAT errors with:

    $ dmesg | grep EtherCAT

1. PDO entry registration failed!

        Pid: 15795
        Failed to register PDO entry: No such file or directory
        PDO entry registration failed!

    Usually occurs when CONFIGURE_PDOS is set to 0 and the appropriate EEPROM is not written to a slave.

