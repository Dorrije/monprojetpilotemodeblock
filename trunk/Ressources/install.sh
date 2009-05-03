#!/bin/sh

echo 
echo Cette application est réalisée par Dhouib Khalil et Neji Wafa
echo GL5 Group 2
echo 


echo Chargement de module en cours..... 
sudo insmod tp2final-driver.ko
echo Module chargé.
echo

echo Création de noeud en cours....

if !( test -e /dev/mondevice) then
sudo mknod /dev/mondevice b 254 0
sudo chmod 666 /dev/mondevice
fi

echo noeud crée avec succès
echo




