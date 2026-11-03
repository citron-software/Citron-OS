# Citron OS

Citron OS est une distribution Linux live amorçable x86_64, basée sur Debian 13. Elle démarre sur un bureau GTK natif compilé pour Linux ; l’ISO ne lance pas `index.html`. Le citron apparaît dans les menus de démarrage et dans un splash Plymouth animé avec des points pulsants et une barre de progression.

## Créer l’ISO

Sur Windows avec WSL et Debian installés, préparez les outils une fois :

```powershell
wsl -d Debian -u root -- apt-get update
wsl -d Debian -u root -- apt-get install -y xorriso squashfs-tools gcc pkg-config libgtk-3-dev libcairo2-dev
```

Puis lancez depuis PowerShell :

```powershell
wsl -d Debian -u root -- bash /mnt/c/Users/ETHAN/Documents/Citron-OS/build-iso.sh
```

Le script vérifie la somme de contrôle de l’image Debian Live officielle, compile `citron-shell.c`, ajoute le bureau et le splash natifs au système live, puis génère `Citron-OS-1.0-amd64.iso` et sa somme SHA-256. L’ISO démarre en BIOS et UEFI ; la session live n’enregistre pas les changements après redémarrage.

## Applications Citron

- Fichiers : explorateur système natif
- Notes : éditeur GTK avec sauvegarde dans Documents
- Internet : navigateur Firefox
- Calculatrice : opérations arithmétiques natives
- Réglages et Terminal : applications Linux

Le bureau fonctionne en session live ; les fichiers et notes créés ne sont pas conservés après redémarrage.
