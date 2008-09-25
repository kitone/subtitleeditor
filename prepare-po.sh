#!/bin/sh
echo '[encoding: UTF-8]' > po/POTFILES.in
find src -name "*.h" | sed -e "s/\.\///g" >> po/POTFILES.in
find src -name "*.cc" | sed -e "s/\.\///g" >> po/POTFILES.in
find share/glade -name "*.glade" | sed -e "s/\.\///g" >> po/POTFILES.in
find share -name "*.desktop.in" | sed -e "s/\.\///g" >> po/POTFILES.in
cat po/POTFILES.in | sort > po/POTFILES.in
