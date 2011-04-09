#!/bin/sh
#Name of the programm without extension
PROGRAM=memorum
#Path to programm's file, relative to install folder
PROGRAMMPATH=../Release/
#Path to install files, like extensions.cfg, localization and extra files
INSTALLPATH=../install
#List of localization files
LANGFILES=??.txt
#List of extra files
EXTRAFILES=share/*
#Path to share folder on device (/mnt/ext1/system/$SHAREPATH)
SHAREPATH=share/$PROGRAM

if [ -z "$PROGRAM" ]; then 
	echo "Select programm name!"
	exit 1
fi

if [ -n "$INSTALLPATH" ]; then
	if [ ! -d $INSTALLPATH ]; then 
		mkdir $INSTALLPATH
	fi
	cd $INSTALLPATH
fi
mv -f $PROGRAMMPATH$PROGRAM.app $PROGRAM.app
tar -cf temp.tar $PROGRAM.app $LANGFILES $EXTRAFILES
mv -f $PROGRAM.app $PROGRAMMPATH$PROGRAM.app

echo -n "#!/bin/sh
SIZE=" > ${PROGRAM}_install.app
du -b temp.tar | cut -f 1 >>${PROGRAM}_install.app
cat >> ${PROGRAM}_install.app <<ENDOFMESSAGE
mkdir /mnt/ext1/games/temp
tail -c \$SIZE \$0 > /mnt/ext1/games/temp/temp.tar
cd /mnt/ext1/games/temp
tar -xf temp.tar
rm -f temp.tar

ENDOFMESSAGE

E=$(find . -name "$LANGFILES" -exec echo -n "0" \;)
if [ -n "$LANGFILES" ] && ([ $E -eq 0 ] || [ -e $LANGFILES ]); then
	echo "for LangFile in \\">> ${PROGRAM}_install.app
	ls -l $LANGFILES | sed -e "s/.* \\(.*\\.txt\\)$/\"\\1\" \\\\/">> ${PROGRAM}_install.app
	cat >> ${PROGRAM}_install.app <<ENDOFMESSAGE

do
	head -1 /ebrmain/language/\${LangFile} > /mnt/ext1/system/language/\${LangFile}.tmp
	cat /mnt/ext1/system/language/\${LangFile} | awk "BEGIN { b = 1 } { if(\\\$0 == \"; Start ${PROGRAM}\") { b = 0 } if(b == 1 && NR != 1) { print \\\$0 } if(\\\$0 == \"; End ${PROGRAM}\") { b = 1 } }"
	echo '; Start '${PROGRAM}>> /mnt/ext1/system/language/\${LangFile}.tmp
	cat \${LangFile} >> /mnt/ext1/system/language/\${LangFile}.tmp
	echo '; End '${PROGRAM}>> /mnt/ext1/system/language/\${LangFile}.tmp
	mv -f /mnt/ext1/system/language/\${LangFile}.tmp /mnt/ext1/system/language/\${LangFile}
	rm -f \${LangFile}>> %PROGRAM%_install.app
done

ENDOFMESSAGE
fi

if [ -e extensions.cfg ]; then
	echo "for Ext in \\">> ${PROGRAM}_install.app
	cat extensions.cfg | sed -e "s/\\(.*\\)/\"\\1\" \\\\/">> ${PROGRAM}_install.app
	cat >> ${PROGRAM}_install.app <<ENDOFMESSAGE

	
do
	echo "^\${Ext}" | head -c 5 > temp_pattern
	cat /mnt/ext1/system/config/extensions.cfg | grep -v -f temp_pattern > /mnt/ext1/system/config/extensions.cfg.tmp
	echo \${Ext} >> /mnt/ext1/system/config/extensions.cfg.tmp
	mv -f /mnt/ext1/system/config/extensions.cfg.tmp /mnt/ext1/system/config/extensions.cfg
done
rm -f temp_pattern
cp -f ${PROGRAM}.app /mnt/ext1/system/bin

ENDOFMESSAGE
fi

E=$(find . -name "$EXTRAFILES" -exec echo -n "0" \;>/dev/null )
if [ -n "$EXTRAFILES" ]; then
	if [ $E -eq 0 ] || [ -e $EXTRAFILES ]; then
		cat >> ${PROGRAM}_install.app <<ENDOFMESSAGE
mkdir /mnt/ext1/system/share
mkdir /mnt/ext1/system/$SHAREPATH
mv -f $EXTRAFILES /mnt/ext1/system/$SHAREPATH

ENDOFMESSAGE
	elif [ -n "$SHAREPATH" ]; then
		cat >> ${PROGRAM}_install.app <<ENDOFMESSAGE
mkdir /mnt/ext1/system/share
mkdir /mnt/ext1/system/$SHAREPATH

ENDOFMESSAGE
	fi
fi

cat >> ${PROGRAM}_install.app <<ENDOFMESSAGE
mv -f ${PROGRAM}.app /mnt/ext1/games
cd ..
rm -fr temp
rm -f \$0
: << 'BINARY_FILE'
Binary starts here:
ENDOFMESSAGE

cat ${PROGRAM}_install.app | sed '' > install.tmp
cat install.tmp temp.tar > ${PROGRAM}_install.app
rm -f install.tmp
rm -f temp.tar
mv -f ${PROGRAM}_install.app ${PROGRAMMPATH}${PROGRAM}_install.app
