s="[Desktop Entry]\nName=海天鹰浮球\nComment=Media player\nExec=`pwd`/HTYFB %u\nIcon=`pwd`/icon.png\nPath=`pwd`\nTerminal=false\nType=Application\nCategories=System;"
echo -e $s > HTYFB.desktop
cp `pwd`/HTYFB.desktop ~/.local/share/applications/HTYFB.desktop