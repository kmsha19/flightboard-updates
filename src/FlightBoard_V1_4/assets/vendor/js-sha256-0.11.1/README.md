# js-sha256 0.11.1

Copie locale non modifiée de [emn178/js-sha256](https://github.com/emn178/js-sha256), version `v0.11.1`, commit figé [`7912494a5ae277660131557401b91f11f47c96d2`](https://github.com/emn178/js-sha256/tree/7912494a5ae277660131557401b91f11f47c96d2). Le tag annoté n'est pas signé ; les fichiers ont été récupérés par HTTPS depuis le dépôt de l'auteur. Chaque URL, taille et empreinte SHA-256 figure dans `manifest.json`.

- `sha256.js` : source lisible officielle.
- `sha256.min.js` : compilation navigateur officielle, utilisée sans modification.
- `LICENSE.txt` : licence MIT complète de l'auteur.

Flight Board utilise la fonction globale `sha256(ArrayBuffer)`, qui renvoie une empreinte hexadécimale. Ce code fonctionne dans la page HTTP locale, où `crypto.subtle` n'est normalement pas disponible. Aucun CDN, paquet téléchargé ni API cryptographique native n'est requis à l'exécution.

`python3 build_assets.py --hash-only` vérifie les trois empreintes et produit `FlightBoardHashGzip.h`. La licence complète est ajoutée en commentaire à l'actif servi par la carte. Le gzip fixe sa date, son nom et son octet de plateforme pour être reproductible. La commande complète `python3 build_assets.py` régénère aussi cet actif ; `--logos-only` le conserve.

Tests hors ligne : `node tests/ui/test_hash.cjs`. Le véritable code navigateur est exécuté dans une VM dépourvue de Node, de Web Crypto et de réseau, puis ses résultats sont comparés aux vecteurs SHA-256 et à l'implémentation native de Node dans le processus de test. L'empreinte détecte un fichier différent du manifeste ; elle ne remplace pas une signature d'éditeur.
