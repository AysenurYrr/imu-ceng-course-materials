#!/bin/bash

for i in $(seq 1 2); do
    gnome-terminal -- bash -c "./drone_client $i 127.0.0.1; exec bash"
done
