#!/bin/sh
echo '[encoding: UTF-8]' > po/POTFILES.in
find share/glade -name "*.glade" | sed -e "s/\.\///g" >> po/POTFILES.in
find share -name "*.desktop.in" | sed -e "s/\.\///g" >> po/POTFILES.in
find src -type f | grep '\.\(h\|cc\)$' | sed -e "s/\.\///g" >> po/POTFILES.in
find plugins -type f | grep '\.\(h\|cc\|glade\)$' | sed -e "s/\.\///g" >> po/POTFILES.in
cat po/POTFILES.in | sort > po/POTFILES.in
