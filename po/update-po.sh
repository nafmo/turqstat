#!/bin/sh

# Update master PO file
cd ..
findtr *.cpp > po/xturqstat.po

# Update translations
cd po
for po in *.po; do
	if [ "$po" != "xturqstat.po" ]; then
		mergetr $po xturqstat.po
	fi
done
