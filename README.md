# Flight Board — mises à jour

by Christ_Kmusic

Ce dépôt distribue les mises à jour de Flight Board pour ESP32-S3. Chaque carte conserve son propre Wi-Fi, ses réglages, ses favoris et son rythme radar mémorisé.

## Depuis la tablette

1. Connecter la tablette au même réseau Wi-Fi que le Flight Board, avec un accès à Internet.
2. Ouvrir [flightboard.local](http://flightboard.local/) ou l’adresse IP de la carte.
3. Aller dans **Réglages → Connexion et mises à jour → Mettre à jour le programme**.
4. Toucher **Rechercher une mise à jour**, puis choisir d’installer la version proposée.

Garder la carte alimentée et la tablette connectée jusqu’au redémarrage. Aucune mise à jour ne s’installe automatiquement.

## Première installation

Une carte neuve nécessite d’abord une installation par USB, avec un partitionnement compatible OTA. Le fichier de ce dépôt contient uniquement l’application ; il sert aux mises à jour d’une carte déjà préparée.

Sur une carte équipée de Flight Board V2.0 à V2.0.7, enregistrer d’abord son réseau dans **Configurer le Wi-Fi** si cela n’a jamais été fait. Télécharger ensuite [Flight Board V2.0.8](releases/FlightBoard-2.0.8-ESP32S3.ino.bin), puis sélectionner ce fichier dans la fenêtre de mise à jour. Les versions suivantes pourront être recherchées directement depuis la tablette.

## Version 2.0.8

- Le radar mémorise son rythme après trois relevés réussis et le retrouve après redémarrage.
- La tablette indique discrètement l’intervalle d’actualisation.
- Les versions publiées ici peuvent être recherchées et installées depuis la tablette.

[Version disponible](latest.json) · [Empreinte SHA-256 du programme V2.0.8](releases/FlightBoard-2.0.8-ESP32S3.ino.bin.sha256)

Les versions distribuées sont compilées sans identifiants Wi-Fi intégrés. Le panneau LED physique reste à intégrer ; cette version utilise l’aperçu dans le navigateur.
