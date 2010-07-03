#!/bin/sh
echo '[encoding: UTF-8]' > po/POTFILES.in
find share/ui -type f | grep '\.ui$' | sed -e "s/\(.*\)/\[type: gettext\/glade\] \1/" >> po/POTFILES.in
find share -name "*.desktop.in" | sed -e "s/\.\///g" >> po/POTFILES.in
find src -type f | grep '\.\(h\|cc\)$' | sed -e "s/\.\///g" >> po/POTFILES.in
find plugins -type f | grep '\.\(h\|cc\|ui\)$' | sed -e "s/\.\///g" >> po/POTFILES.in
find plugins -type f | grep '\.se-plugin\.in$' | sed -e "s/\(.*\)/\[type: gettext\/ini\] \1/" >> po/POTFILES.in
find plugins -type f | grep '\.se-pattern\.in$' | sed -e "s/\(.*\)/\[type: gettext\/xml\] \1/" >> po/POTFILES.in
find plugins -type f | grep '\.ui$' | sed -e "s/\(.*\)/\[type: gettext\/glade\] \1/" >> po/POTFILES.in
sort po/POTFILES.in -o po/POTFILES.in
