# Adeptus

Adeptus is an issue tracking application I've been using for personal projects such as [reZonator](https://github.com/orion-project/rezonator2) or [Procyon](https://github.com/orion-project/procyon) or Adeptus itself.

Adeptus manages project issues storing them in a single SQLite database. It doesn't use its own server, and you are free to choose favored sync service, like Dropbox or Google Drive, to share the database between your working machines.

## Screenshot

Sorry for that, I'm tracking my issues in Russian, but you can get a clue anyway :) 

![Main Window](./img/screen/main_window.png)

## Build

Clone the repository

```bash
git clone https://github.com/orion-project/adeptus
```

Update submodules:

```bash
cd adeptus
git submodule init
git submodule update
```

Note, that submodules are in 'detached head' state by default.
