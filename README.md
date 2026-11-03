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

Le script vérifie la somme de contrôle de l’image Debian Live officielle, compile `citron-shell.c`, ajoute le bureau natif, les applications GNOME et le splash Plymouth animé, puis génère `Citron-OS-1.1-amd64.iso` et sa somme SHA-256. L’ISO démarre en BIOS et UEFI.

## Session persistante sur clé USB

Le démarrage passe le paramètre `persistence` à Debian Live. Pour conserver les réglages, créez sur la clé une partition ext4 portant l’étiquette `persistence`, puis placez-y un fichier `persistence.conf` contenant `/ union`. Cette partition conserve aussi le profil et le mot de passe système configuré par l’assistant (celui-ci est enregistré par Debian sous forme de hash dans `/etc/shadow`, jamais en clair). Sans cette partition préparée, une session live reste temporaire.

Au premier lancement du bureau, l’assistant propose le nom affiché, le mot de passe, la langue française/anglaise et la disposition AZERTY/QWERTY. Le profil est enregistré sous `~/.config/citron-os/profile.ini`.

## Applications Citron

- Fichiers : explorateur système natif
- Notes : éditeur GTK avec sauvegarde dans Documents
- Firefox : navigateur Web
- Calculatrice : opérations arithmétiques natives
- Paramètres GNOME, Terminal GNOME, Calendrier, Horloge et Firmware sont inclus dans l’image. « Pilotes additionnels » ouvre GNOME Firmware pour les périphériques compatibles fwupd. « Install to Hard Drive » lance Calamares, inclus dans l’image.

Le menu Démarrer présente une recherche et une grille d’applications inspirées de Windows 10.
