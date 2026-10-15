# Ressources de ZCS Contrôle V4.1

Les images originales sont conservées dans `sources/`, leurs URL et empreintes SHA-256 dans `airlines.json`. Ce manifeste contient 36 marques et 45 préfixes ICAO. Les marques appartiennent à leurs titulaires ; ces images servent à identifier les compagnies dans le tableau de vols.

## Provenance

- Air Côte d’Ivoire : [icône officielle du site](https://www.aircotedivoire.com/wp-content/uploads/2023/06/cropped-favicon-1-192x192.png).
- Corsair : [icône officielle du site](https://cdn.flycorsair.com/cdn-corsair-front-dev/booking-engine/favicon/favicon-96x96.png).
- Autres emblèmes : collection [Jxck-S/airline-logos](https://github.com/Jxck-S/airline-logos/tree/e0b218991caef4832e0db911136173e8eead942a), révision figée `e0b218991caef4832e0db911136173e8eead942a`, sources FlightAware et RadarBox, URL exacte par image dans le manifeste.
- Brussels : neuf points rouges, identité vérifiée avec l’[annonce officielle](https://press.brusselsairlines.com/brussels-airlines-confirms-its-position-in-the-market-with-a-new-brand-identity) et le [logo du site actuel](https://www.brusselsairlines.com/etc/designs/sn/clientlib-all/vite/img/brussel-logo.svg).
- Ethiopian : emblème tricolore comparé au [logo du site corporate](https://corporate.ethiopianairlines.com/images/default-source/star-alliance-airlines/et-logo-for-web.jpg?sfvrsn=dbe31d20_2).
- Lufthansa : silhouette transparente conservée, rendue en blanc pour le panneau noir, comme la [variante négative du catalogue](https://raw.githubusercontent.com/Jxck-S/airline-logos/e0b218991caef4832e0db911136173e8eead942a/radarbox_logos/DLH.png). Cette exception est indiquée par `dark_variant` et sa référence dans le manifeste.
- `DEFAULT.png` : pictogramme avion neutre issu de la version précédente du projet.

Les codes prioritaires ont été vérifiés dans la [liste ICAO CORSIA](https://www.icao.int/CORSIA/AeroplaneOperators). Les alias Ryanair UK (RUK), Malta Air (MAY) et DHL (DHA/DHK/DHV) sont vérifiables dans l’[annuaire des indicatifs FAA](https://www.faa.gov/air_traffic/publications/atpubs/cnt_html/chap3_section_1.html). Les alias sont regroupés par marque affichée, pas par raison sociale. Un préfixe absent conserve le pictogramme neutre ; le système n’invente pas de compagnie depuis un code IATA ou une immatriculation.

## Génération hors ligne

Installer Python 3 et Node.js 18 ou ultérieur, puis les dépendances figées depuis le projet :

```sh
python3 -m pip install Pillow==12.3.0 zopfli==0.2.3.post1
npm ci --ignore-scripts
python3 build_assets.py
```

Le script vérifie les empreintes des originaux, enlève seulement leurs marges transparentes et conserve les proportions. Les PNG de la tablette gardent leur transparence ; les pixels transparents deviennent noirs, donc éteints, sur le panneau. Aucun rectangle blanc ou bleu n’est ajouté. Les blancs internes (croix Swiss, voile Corsair, fleur Tahiti) et les fonds qui font partie du logo (easyJet orange, DHL jaune) sont préservés. Aucune couleur blanche n’est supprimée automatiquement. Il produit :

- `AirlineLogos.h` : pixels RGB565 24×16 en palettes triées à deltas LEB128 et indices avec plages noires/répétitions sans perte, et offsets vers le pack WebP externe ; les alias partagent les mêmes images. `readAirlinePixel()` décode un pixel RGB565 sans tampon supplémentaire.
- `tablet/*.png` : PNG à palette de 32 couleurs avec transparence, conservés comme référence. `/api/logo/image?flight=...` sert leur équivalent WebP sans perte depuis la mémoire locale après installation du pack de logos. Avant cette installation, la tablette utilise le logo RGB565 local. La génération compare tous les pixels RGBA avant et après conversion.
- `../logo_*.png` : agrandissements des pixels LED réels et `../PLANCHE_LOGOS.png` comparant tablette et panneau.
- `zcs-logos.bin/json` : 37 WebP concaténés et leur table de provenance (offsets, tailles, SHA-256). Les pixels et leur transparence sont inchangés.
- `zcs-web.bin/json` : page `FlightBoardWeb.h` avec son JavaScript inline minifié, puis compressée en gzip et destinée à la partition de contenus. Le HTML, les attributs, le CSS et les références aux scripts externes restent identiques.
- `../FlightBoardWebGzip.h` : taille, SHA-256 et URL versionnée du pack web, sans le gros tableau HTML.
- `../ZcsBootstrap.h` : page de secours embarquée générée depuis `bootstrap.html`, limitée à 4 Kio gzip. Elle permet le téléchargement vérifié ou l’import local de l’interface, la configuration Wi-Fi et une mise à jour manuelle du programme.

Régénérer après chaque modification de `FlightBoardWeb.h`, du manifeste ou d’un original. L’option `--logos-only` permet de régénérer les images sans toucher à l’interface compressée. Les fichiers générés sont fournis ; Pillow avec prise en charge WebP, Zopfli et esbuild sont requis pour les régénérer, pas pour compiler une version inchangée. Les tests hôtes refusent une interface gzip qui ne correspond plus à la transformation déterministe de sa source et vérifient les marges noires, la transparence et les détails blancs internes. Depuis V3.0.0, l’empreinte des 384 pixels de chacun des 37 emblèmes est comparée à la référence V2.0.10. Les anciens `APERCU_PANNEAU_*.png` restent des exemples historiques ; l’aperçu navigateur est la référence actuelle.

Depuis V4, `tools/minify_web.cjs` utilise **esbuild 0.28.2**, figé dans `package.json` et `package-lock.json`, uniquement pendant la génération. Seuls les espaces et les identifiants locaux du script applicatif sont réduits ; les optimisations de syntaxe, la suppression de code et le renommage des propriétés sont désactivés. Le script est isolé dans une fonction IIFE ; ses identifiants applicatifs sont locaux et peuvent être compactés. Les références aux scripts externes et les propriétés conservent leur nom. Les gestionnaires HTML inline sont refusés pour éviter une dépendance aux anciennes fonctions globales. Un nombre inattendu de scripts inline, une erreur, un avertissement ou une autre version du compilateur bloque la génération. `FlightBoardWeb.h` reste la source lisible et modifiable ; aucun outil npm ni CDN n’est chargé par le panneau ou la tablette.

La compression Zopfli produit ensuite du gzip standard et conserve exactement cette sortie de compilation ; elle ne s’exécute que sur l’ordinateur pendant la génération. `--hash-only` reste sans dépendance externe, y compris sans Node.js. Les images WebP utilisent un effort de compression de 100, toujours sans perte.

Le pack web publié suit une URL versionnée (`store/zcs-web-4.1.0.bin`) : ne jamais remplacer ses octets après publication. Le firmware accepte seulement son empreinte compilée. `--bootstrap-only` régénère la page de secours depuis le manifeste web existant, avec la seule bibliothèque standard Python. Après modification du code applicatif, utiliser la génération complète.
