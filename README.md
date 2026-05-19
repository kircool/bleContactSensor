Ble contact senosor based on esp32-c3 (fully vibecoded)

Project for PlatformIO.

You can use different esp32 model, but you might have to change code part about depp sleep as it differ for esp variants.

Uses GPIO4

Algoritm:

power on

send status

go to deep sleep

on event 

  rise up
  
  send status
  
  go to deep sleep 

You'll need second esp32 to capture packets. See esphome_example.yaml

