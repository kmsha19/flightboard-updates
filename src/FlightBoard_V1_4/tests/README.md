# Vérifications sur ordinateur

Depuis le dossier du projet :

```sh
python3 tests/host/run_tests.py .
```

Prérequis : Python 3, Node.js 18 ou ultérieur avec `npm ci --ignore-scripts`, `clang++` ou `g++`, et ArduinoJson installé dans
`../libraries/ArduinoJson/src`. Pour une autre installation :

```sh
python3 tests/host/run_tests.py . --arduino-json /chemin/ArduinoJson/src
```

Les tests compilent les vrais en-têtes du firmware et extraient directement les
fonctions de réglages du fichier `.ino`. Les périphériques, le réseau, la mémoire
persistante et les écritures OTA sont simulés. Aucune connexion réseau ni écriture
sur l’ESP32 n’est effectuée ; `Secrets.h` n’est pas lu. Les fichiers générés restent
dans un dossier temporaire automatiquement supprimé.

## V4 : vérification de l’interface embarquée

`node tests/ui/test_web_build.cjs` vérifie que la minification déterministe utilise esbuild 0.28.2, conserve exactement le HTML/CSS et tous les attributs du DOM, préserve les fonctions globales, propriétés et textes Unicode, et refuse une source ou une dépendance inattendue. Le gzip réellement livré doit être identique à la transformation de la source lisible actuelle et contenir un script JavaScript valide, sans horodatage de compilation. La suite hôte applique également cette vérification de fraîcheur. Les tests de comportement continuent de lire `FlightBoardWeb.h` ; les essais navigateur utilisent le gzip servi par la carte.

Les suites couvrent les favoris et leurs erreurs de sauvegarde, les horaires et
changements d’heure, la migration V1.9, le traitement du radar et les files de
travail, la récupération Wi-Fi, le blocage des changements Wi-Fi pendant une mise
à jour, ainsi que le contrôle des fichiers OTA et les transferts interrompus.

Ces tests complètent la compilation ESP32-S3 et les essais de l’interface dans un
navigateur. Ils ne remplacent pas une vérification sur la carte et le panneau LED.

La validation V2.0.8 compte **26 923 assertions hôtes**, dont **755 pour le radar**.
Cette suite distingue les erreurs HTTP et réseau, les réponses vides, incomplètes
ou invalides, les dépassements de taille et les délais de lecture. Elle vérifie
aussi l’intervalle initial de 15 secondes et l’attente de 120 secondes après HTTP 429, les réponses lentes ou tronquées, le
passage à zéro du compteur de temps, les distances en milles nautiques de l’API,
le filtrage exact en kilomètres et la disponibilité du traitement radar. Une
régression reproduit les pauses entre paquets TLS du client ESP32, avec une
connexion toujours ouverte : la lecture doit reprendre, tandis qu’une vraie fin
de connexion tronquée doit échouer et qu’une connexion bloquée doit expirer.

Les diagnostics affichés sur la tablette se vérifient séparément :

```sh
node tests/ui/test_radar_status.cjs
```

Ses **256 assertions** couvrent les messages de démarrage, relevé, pause, HTTP
429/5xx, connexion, lecture et mémoire, ainsi que le compte à rebours sans valeur
négative. Elles vérifient que les avions périmés disparaissent du radar, de la
fiche et de l’aperçu LED, tandis qu’un relevé encore récent reste utilisable pendant
la requête suivante. Une pause volontaire de configuration Wi-Fi affiche
« Radar suspendu » avec une pastille ambre, même si le dernier relevé est frais ;
elle reste distincte d’une véritable déconnexion Wi-Fi. Les anciens firmwares,
le mode démonstration et la différence entre ciel vide et relevé en échec sont
également couverts.

La recherche de trajets VRS / ADSB.lol fait partie des suites hôtes (**324 assertions** : indicatif, ordre des aéroports, certificat public GTS Root R4, pauses TLS, délais, cache et cohérence géographique). Le cas RYR3UF vérifie que le trajet Nantes–Valence reste plausible et qu’un trajet Banja Luka–Berlin est masqué pour un avion situé dans l’ouest de la France. Les tests couvrent aussi les longs courriers, le changement de date et les virages près des aéroports. Le JavaScript de l’interface dispose aussi d’un test sans dépendance :

```sh
node tests/ui/test_routes.cjs
```

Il vérifie notamment les affichages connu/inconnu/erreur et le rejet des réponses arrivées après un changement de vol.

Agencement du panneau 128×64 : `node tests/ui/test_preview.cjs` vérifie aussi les positions des textes, leurs limites et les mesures distance/vitesse/altitude.

Logos : `node tests/ui/test_logos.cjs` vérifie le fallback HTML, le cache versionné et les réponses arrivant après un changement de sélection. La suite hôte vérifie les préfixes ICAO, les alias, les PNG et le dessin RGB565. Elle vérifie aussi que `FlightBoardWebGzip.h` correspond exactement au HTML source ; si nécessaire, régénérer avec `python3 build_assets.py` (Pillow 12.3.0).

Horloge animée : `node tests/ui/test_idle.cjs` couvre **115 assertions** : mouvement déterministe, contraste de l’heure, cadence, ciel vide, mode Horloge, absence d’heure synchronisée, luminosité et veille, aperçu replié, onglet masqué et mouvement réduit. Les six suites JavaScript totalisent **576 assertions**. Leurs vérifications de géométrie sont regroupées par rendu.

Police bitmap : `node tests/ui/test_fonts.cjs` vérifie **81 assertions** sur les glyphes 5×7 et 3×5, les pixels entiers sans lissage, la largeur des trajets et de l’horloge, les caractères inconnus et le traitement des valeurs trop longues.

Cadence adaptative : la suite radar couvre les réponses 200/429 intermittentes, la départ à 15 secondes, tous les paliers de 5 secondes jusqu’à 120 secondes et leur conservation après succès, la pause obligatoire et le débordement du compteur. `Retry-After` est honoré au format secondes ; une date HTTP ou valeur invalide conserve la pause minimale de 120 secondes. La pause active reste visible dans l’API après changement de lieu.


## V2.0.8 : cadence persistante et mises à jour distantes

Le radar teste la restauration des valeurs valides, le rejet des valeurs NVS corrompues ou hors limites, les trois succès exploitables nécessaires, les interruptions de validation, la limitation des écritures et leur reprise après échec. Les tests Wi-Fi simulent une mise à jour publique sans identifiants intégrés et vérifient que le réseau hérité est restauré depuis NVS ; l’OTA est refusée si sa sauvegarde échoue.

```sh
node tests/ui/test_updates.cjs
node tests/ui/test_hash.cjs
```

Les mises à jour distantes ont 118 assertions de flux : manifeste et version, famille, origine/répertoire HTTPS, tailles bornées, téléchargement interrompu, timeout, empreinte incorrecte, fermeture pendant une recherche, compatibilité revue juste avant installation, repli manuel et version réelle après redémarrage. Les huit suites UI totalisent 970 assertions standard.

Le SHA-256 embarqué est exécuté dans un contexte navigateur sans Web Crypto ; source, minifié et gzip sont comparés aux vecteurs attendus et au résultat Node Crypto. Les tests OTA peuvent également recevoir en arguments l’application réelle, le bootloader et l’image merged, afin de confirmer que seule l’application complète est acceptée.

Le canal public kmsha19/flightboard-updates a été publié et vérifié le 15 octobre 2026 : manifeste et binaire HTTP 200 en HTTPS, CORS autorisé, fichier téléchargé de taille et SHA-256 conformes. Sur la vraie carte V2.0.8, la recherche affiche « Votre Flight Board est à jour ». Aucune nouvelle installation n’est déclenchée lorsque la version publiée est déjà installée ; les branches d’installation et d’erreur du client sont couvertes par les tests simulés.


Le paquet public se vérifie avec `python3 -B tests/release/test_prepare_release.py -v` : provenance de la compilation, refus de Secrets.h sans lire son contenu, structure et empreinte d’image complètes, immutabilité des versions existantes, absence de fichiers étrangers/symlinks et génération déterministe. La vérification accepte seulement le préfixe `#line` exact ajouté par Arduino pour le fichier public attendu.

## V2.0.9 : premier panneau physique

`node tests/host/test_panel_parity.cjs` compile le renderer C++ autonome et compare huit scènes à l’aperçu JavaScript : 8 192 pixels RGB565 identiques par scène. La suite C++ couvre 10 616 assertions, notamment données périmées, modes, logos, métriques, trajets non confirmés, horloge, absence d’heure et veille. Le wrapper matériel est compilé avec le pilote officiel épinglé Waveshare et le cœur ESP32 3.3.12. Ces tests ne prouvent pas le câblage ou le rendu optique.

Le panneau démarre éteint puis propose brièvement trois carrés rouge, vert et bleu. La luminosité est plafonnée à 10 % pour cette mise en service. Les télémetries `display.panelReady`, `panelFrames`, `panelRenderMicros`, `panelMinFreeHeap` et `panelLargestDmaBlock` permettent de contrôler le pilote et la marge mémoire sur la carte pendant les relevés réels. Les trajets utilisés par le renderer passent par le même filtre que l’API ; la régression RYR3UF vérifie aussi cet accès direct.

Le paquet public compare désormais les sources C++ imbriquées et les métadonnées du pilote, en plus des fichiers racine. La V2.0.9 de test reste locale ; le canal GitHub demeure en V2.0.8 pendant la validation physique.

## V2.0.10 : type et correspondance RGB

Les tests de panneau couvrent le type d’appareil sur une seconde ligne sans chevauchement du logo, de l’indicatif ou du trajet. Quinze scènes C++/JS ont des pixels RGB565 identiques. Codes 2–4 caractères connus, absence et valeurs invalides (espaces, contrôle, sauts de ligne et codes trop longs) sont vérifiés : 21 890 assertions C++ et 359 assertions d’aperçu.

La permutation des six sorties couleur est déduite de la mire réelle confirmée par l’utilisateur (rouge→bleu,vert→rouge,bleu→vert), avec revue de l’inverse appliqué aux deux moitiés du panneau. Les assets et le renderer restent RGB standards. Après installation de la V2.0.10 le 15 octobre 2026, l’utilisateur a confirmé que les couleurs sont correctes sur le panneau physique.

## ZCS CONTROL : applications, filtres et météo

La suite hôte comprend maintenant **27 305 assertions**. Les réglages vérifient
la migration sans modification des anciennes préférences, les applications et
ambiances autorisées, les types et bornes des filtres, ainsi que les sauvegardes
atomiques. Les vrais gestionnaires d’API sont exercés pour une activation
application + ambiance en une écriture et pour le retrait d’un vol épinglé
exclu par les filtres, y compris quand l’enregistrement échoue.

Les **204 assertions météo** utilisent uniquement un service HTTP simulé : unités,
codes WMO, dates, limites numériques, certificat configuré, réponses bornées,
pauses réseau, délais, cadence de 15 minutes, indication ancienne après 30 minutes,
conservation du dernier relevé après échec, changement de lieu et libération de
la tâche après chaque requête. Elles ne prouvent pas la disponibilité du service
réel sur le réseau de l’utilisateur.

`node tests/host/test_panel_parity.cjs` vérifie **52 scènes** avec **8 192 pixels
RGB565 identiques** chacune et **21 929 assertions C++** : Flight Board, les quatre
ambiances avec et sans heure, animation arrêtée, météo, données manquantes ou
anciennes et veille nocturne. Les neuf suites `tests/ui/*.cjs` comptent **1 380
assertions**, dont `test_zcs.cjs` pour la navigation sans activation implicite,
le choix local d’ambiance, les filtres et les erreurs de sauvegarde. Cette suite
compte 106 assertions : l’âge météo continue d’augmenter hors ligne, un relevé
ancien est signalé, les arrondis négatifs correspondent au panneau et les filtres
de démonstration sélectionnent uniquement un vol admissible, y compris après
le retrait d’un vol épinglé ou un retour en mode Auto.

La compression des logos est vérifiée séparément : les 37 empreintes RGB565
de référence V2.0.10 sont conservées après décodage palette/RLE. Avec Pillow,
`python3 tests/test_logo_images.py` décode les 37 WebP embarqués et compare
l’alpha et chaque pixel visible aux PNG sources conservés dans `assets/tablet`.


## V4 : bibliothèque et interface

`node tests/ui/test_library.cjs` vérifie 331 assertions : 16 contenus, catégories et recherche sans accents, favoris robustes aux erreurs de stockage, activation atomique, mode hors ligne, inspection sans commande, aperçu indépendant et focus clavier conservé. `test_zcs.cjs` comporte 116 assertions, incluant le maintien du brouillon lors de réponses tardives et lors de l’inspection d’une autre ambiance de l’application active.

Les douze suites UI totalisent 1 956 assertions, dont 31 sur la minification déterministe. Le JavaScript lisible et le HTML exact extrait du gzip ont également été exercés dans le navigateur aux dimensions de la tablette ; aucun appel d’activation n’est produit par une simple consultation.
