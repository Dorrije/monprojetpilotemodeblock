#!/bin/sh

if ([ $# -lt 1 ]) then
echo  "Veuiller choisir -r pour la lecture ou -w pour l'ecriture"
exit 0
fi

if ([ $# -gt 2 ]) then
echo  "Nombre d'arguments incorrect"
exit 0
fi


if ([ $1 = -r  -a  $# -eq 1 ]) then
echo
echo Lecture à partir de fichier:
echo
cat /dev/mondevice
exit 1
elif ([ $1 = -r  -a  $# -gt 1 ]) then
echo "Nombre d'arguments incorrects"
fi

if ([ $1 = -w -a   $# -eq 1 ]) then
echo
echo "Entrer la chaine à saisir>"
read chaine
echo "$chaine" > /dev/mondevice
echo "L'ecriture dans le fichier est efféctuée!"
#cat /dev/mondevice
exit 1

elif ([  $1 = -w   -a   $# -eq 2  ]) then
echo
echo "$2" > /dev/mondevice
echo "L'ecriture dans le fichier est efféctuée!"
#cat /dev/mondevice
exit 1

fi
fi



