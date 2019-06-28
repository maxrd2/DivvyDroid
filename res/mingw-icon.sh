#!/bin/bash

for s in 16 32 48 256 ; do 
	sed -Ee 's/(width|height)="[0-9]+"/\1="'$s'"/g' icons/android-debug-bridge.svg \
	| convert -background none svg:- -scale $s icons/android-debug-bridge$s.png
done
icotool -c icons/android-debug-bridge{16,32,48,256}.png -o icons/divvydroid.ico
rm icons/android-debug-bridge{16,32,48,256}.png

