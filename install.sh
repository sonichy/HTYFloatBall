s="[Desktop Entry]\nName=海天鹰浮球\nComment=托盘显示内存使用率，浮窗显示网速。\nExec=`pwd`/HTYFB\nIcon=`pwd`/HTYFB.png\nPath=`pwd`\nTerminal=false\nType=Application\nCategories=System;"
echo $s > HTYFB.desktop
cp `pwd`/HTYFB.desktop ~/.local/share/applications/HTYFB.desktop