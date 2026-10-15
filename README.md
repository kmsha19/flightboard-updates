# ZCS CONTRÔLE · V4.0.0

**by Christ_Kmusic** — une bibliothèque de contenus pour votre panneau LED, pilotée depuis une tablette ou un téléphone.

![Bibliothèque ZCS CONTRÔLE V4 — aperçu local](docs/zcs-control-v4.jpg)

## La bibliothèque

La V4 repense entièrement l’interface : navigation latérale, espace clair, aperçu du panneau, recherche et catégories. Les **16 contenus** regroupent Flight Board, Météo, sept horloges et sept animations. Les étoiles créent une collection de favoris enregistrée dans le navigateur de cette tablette.

**Découvrir** ouvre un aperçu sans interrompre le panneau. **Afficher** active le contenu et son ambiance ensemble. Les favoris, la recherche et les aperçus n’envoient aucune commande au panneau. Un choix consulté pendant une requête en cours reste conservé.

- **Flight Board** : radar, vols proches, indicatif, compagnie, logo, type d’appareil, trajet lorsqu’il est disponible, distance, vitesse et altitude. Filtres combinables par logo disponible, compagnie, type et altitude maximale. Le trajet reste indicatif et peut être absent lorsque sa cohérence n’est pas confirmée.
- **Météo** : température, vent et illustration animée selon les conditions. Le dernier relevé est daté ; une donnée indisponible n’est pas remplacée par une valeur inventée.
- **Horloge et Animations** : Aurore, Océan, Hyperespace, Lave, Étoiles, Matrix et Minimal / Écran noir. Les animations sont générées localement, sans téléchargement de vidéo.
- **Réglages** : luminosités de jour et de nuit immédiates et mémorisées, horaires de nuit, fuseau horaire, lieu, Wi-Fi et mise à jour depuis la tablette.

L’interface fonctionne avec ses ressources embarquées, sans police ou bibliothèque distante. Elle est adaptée notamment aux tablettes **800 × 1340** et **1340 × 800**. L’animation de l’aperçu respecte la préférence de réduction des mouvements du navigateur.

## Matériel de cette version

Le binaire fourni est destiné à la **Waveshare ESP32-S3-RGB-Matrix avec un panneau HUB75 K716 128 × 64, balayage 1/32**. La correspondance des couleurs est calibrée pour le panneau K716-128X64-32S V3.3 validé sur ce projet. Pour un câblage ou un panneau différent, adapter le pilote puis compiler les sources.

| Signaux | Broches |
| --- | --- |
| R1 / G1 / B1 | 5 / 6 / 4 |
| R2 / G2 / B2 | 15 / 16 / 7 |
| A / B / C / D / E | 18 / 8 / 3 / 42 / 9 |
| CLK / LAT / OE | 41 / 40 / 2 |

La luminosité est réglable de 10 à 100 %. Pendant la plage de nuit, le panneau applique le minimum des luminosités de jour et de nuit, ou s’éteint si la veille est choisie. Changer le curseur nocturne en journée prépare la prochaine nuit. L’activation de la plage et ses horaires nécessitent **Enregistrer les réglages**.

## Installer la V4

Sur une carte déjà configurée et compatible : ouvrir **http://flightboard.local/**, puis **Réglages → Connexion et mises à jour → Mettre à jour le programme → Rechercher une mise à jour**. Vérifier la compatibilité matérielle ci-dessus et lancer l’installation. La carte redémarre ; recharger ensuite la page pour recevoir la nouvelle interface.

Les réglages, le Wi-Fi, les lieux favoris et la cadence radar restent en mémoire. La mise à jour ne change pas les partitions. Le téléchargement passe par HTTPS, puis sa taille et son SHA-256 sont vérifiés avant l’installation sur la partition inactive.

- [Programme V4.0.0](releases/ZCS-Control-4.0.0-Waveshare-K716.ino.bin)
- [Empreinte SHA-256](releases/ZCS-Control-4.0.0-Waveshare-K716.ino.bin.sha256)
- [Manifeste de mise à jour](latest.json)
- [Sources complètes](src/FlightBoard_V1_4)
- [Inventaire SHA-256 des sources](source-manifest.json)

Le fichier `.ino.bin` est l’application OTA. Ne pas envoyer un bootloader ou une image « merged » par le formulaire de mise à jour. Pour une première installation USB, compiler et téléverser le sketch avec Arduino IDE ou Arduino CLI. Le programme public ne contient aucun identifiant Wi-Fi ; la configuration se fait depuis le réseau de secours de la carte. Au premier démarrage, ouvrir le moniteur série USB à **115200 bauds** pour lire le réseau `FlightBoard-XXXX` et sa « Clé de secours », rejoindre ce réseau puis ouvrir **http://192.168.4.1/** pour choisir le Wi-Fi. Garder cette clé privée.

## Compiler les sources

Le dossier du sketch conserve le nom `FlightBoard_V1_4` pour la compatibilité Arduino ; la version du programme est bien **4.0.0**.

Dépendances utilisées : cœur **ESP32 3.3.12**, **ArduinoJson 7.4.3**. Le pilote HUB75 Waveshare est embarqué et épinglé sous `assets/vendor/waveshare-hub75`. Les fichiers générés sont inclus : Node et Python ne sont nécessaires que pour régénérer les ressources après une modification.

```sh
cd src/FlightBoard_V1_4
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install ArduinoJson@7.4.3
arduino-cli compile --fqbn 'esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc' --build-path /tmp/zcs-control-build .
```

Après modification de l’interface ou des logos, utiliser Node.js 18 ou supérieur et Python 3.10 ou supérieur, de préférence dans un environnement virtuel, avec les dépendances exactes :

```sh
npm ci --ignore-scripts
python3 -m pip install Pillow==12.3.0 zopfli==0.2.3.post1
python3 build_assets.py
```

`FlightBoardWeb.h` reste lisible. La génération compacte seulement les espaces et les identifiants JavaScript locaux avec **esbuild 0.28.2**, puis compresse l’interface avec Zopfli. Le HTML, le CSS, les noms globaux et les propriétés sont conservés ; aucune compilation ne s’effectue sur la carte. `--hash-only` fonctionne avec la bibliothèque standard Python.

Pour un binaire public, compiler une copie du sketch **sans `Secrets.h`**. `tools/prepare_release.py` contrôle la provenance de la compilation et refuse ce fichier par son nom, sans lire son contenu. Le dossier de compilation publique doit contenir uniquement les fichiers racine `.h`, `.cpp` et `.ino` (sauf `Secrets.h`), ainsi que `assets/vendor/waveshare-hub75` ; les tests et outils restent dans le dépôt source. Ne pas ajouter d’identifiants privés au dépôt.

## Vérifier

```sh
npm ci --ignore-scripts
for suite in tests/ui/*.cjs; do node "$suite"; done
python3 tests/host/run_tests.py . --arduino-json /chemin/vers/ArduinoJson/src
node tests/host/test_panel_parity.cjs
python3 -B tests/release/test_prepare_release.py -v
```

Les tests couvrent la bibliothèque, les réponses tardives, les favoris, les aperçus indépendants, les curseurs de luminosité, l’aviation, les mises à jour et le programme embarqué. La comparaison C++/JavaScript vérifie 320 scènes du panneau, pixel par pixel. Le rendu optique et le câblage nécessitent toujours une vérification sur le matériel correspondant.

## Données et ressources

- Les positions viennent d’ADSB.lol ; les trajets indicatifs sont recherchés via VRS. Le radar mémorise le rythme qui fonctionne et augmente son délai lors d’une limitation HTTP 429. Les nouvelles recherches sont suspendues hors aviation.
- La météo utilise Open-Meteo avec les coordonnées du lieu arrondies à deux décimales, environ 1 km. Elle est relevée toutes les 15 minutes lorsque le module est actif. Un relevé de plus de 30 minutes est signalé comme ancien.
- Les logos conservent leurs sources, références et licences dans [assets](src/FlightBoard_V1_4/assets). Les marques restent la propriété de leurs titulaires. Les licences des composants embarqués sont conservées dans leurs dossiers `vendor`.

Les scores sportifs, messages et playlists sont des pistes futures ; ils ne font pas partie de cette version.
