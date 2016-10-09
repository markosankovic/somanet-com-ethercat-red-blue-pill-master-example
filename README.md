SOMANET COM EtherCAT Red Blue Pill Master Example
=================================================

This is the EtherCAT master example program for [SOMANET CORE C22](https://doc.synapticon.com/hardware/core-c22/revision-a5/index.html) and [SOMANET COM EtherCAT](https://doc.synapticon.com/hardware/com-ethercat/revision-a4/index.html) stack from [Synapticon](https://www.synapticon.com/).

This program sends the PDO named "Offered pill" to slave and receives the "Pill taken" PDO. The value of "Offered pill" PDO can be changed by sending the appropriate signal to the running process.

How to run
----------

    $ siitool -o eeprom.bin esi.xml
    $ ethercat sii_write eeprom.bin
    $ sudo service ethercat restart # restart is required after eeprom write
    $ make
    $ ./somanet_com_ethercat_red_blue_pill_master_example

Example of changing the "Offered pill" PDO value:

    $ kill -SIGUSR1 15404 # send red pill
    $ kill -SIGUSR2 15404 # send blue pill
