# flipper-deploy

Déploiement de la stack **Flipper Virtuel** sur une borne **Fliphetic**.

Ce repo ne contient **pas de code** : seulement le manifeste Fliphetic et un
Docker Compose qui tire des **images déjà publiées** (aucun build sur la borne).
Le code vit dans `Flipper_front` et `Flipper_back`.

## Ce que ça lance

| Service | Image | Rôle | Exposé |
|---------|-------|------|--------|
| `playfield` | `ghcr.io/fouuuadi/flipper-front-playfield` | Écran de jeu 3D | port éphémère |
| `backglass` | `ghcr.io/fouuuadi/flipper-front-backglass` | Score / animations | port éphémère |
| `dmd` | `ghcr.io/fouuuadi/flipper-front-dmd` | Dot-matrix | port éphémère |
| `backend` | `ghcr.io/fouuuadi/flipper-backend` | API REST + WebSocket + MQTT | interne |
| `db` | `postgres:16-alpine` | Persistance scores | interne |
| `redis` | `redis:7-alpine` | Sessions live | interne |
| `mqtt` | `eclipse-mosquitto:2` | Broker IoT | interne |

Les 3 fronts proxifient `/api` et `/ws` vers `backend` (même origine), donc
seuls eux exposent un port — la borne y branche ses 3 écrans Chromium.

## Variables d'environnement

Au **Load**, Fliphetic écrit un `docker-compose.override.yml` qui pose les
`environment:` des conteneurs (cf. `fliphetic/docs/architecture.md`). C'est un
**merge d'override**, pas de l'interpolation : le compose de base ne doit donc
**pas** utiliser `${VAR}` pour ces variables (l'interpolation tourne au parse,
avant l'override). Conséquence :

- **Config + secret** (`POSTGRES_*`, `DB_*`, `LOG_LEVEL`) → **dans le dashboard**,
  pas dans le repo. Le compose de base ne les déclare pas.
- **Topologie réseau** (`DB_HOST=db`, `DB_PORT`, `REDIS_URL`, `MQTT_BROKER_*`,
  `APP_PORT`, `REDIS_SESSION_TTL_SECONDS`, `MQTT_TOPIC_FILTER`) → **littéral** dans
  le compose : c'est le câblage du réseau Docker, pas de la config.

Variables à créer dans le dashboard (l'image postgres lit `POSTGRES_*`, le backend
lit `DB_*` → le mot de passe est sous deux noms, même valeur) :

| Variable | Conteneur | Valeur |
|----------|-----------|--------|
| `POSTGRES_DB` | db | `flipper` |
| `POSTGRES_USER` | db | `flipper_user` |
| `POSTGRES_PASSWORD` | db | *(secret)* |
| `DB_NAME` | backend | `flipper` |
| `DB_USER` | backend | `flipper_user` |
| `DB_PASSWORD` | backend | *(même secret)* |
| `LOG_LEVEL` | backend | `INFO` |

> Tu peux aussi les laisser toutes en *all containers* (chaque conteneur ignore
> celles qu'il ne lit pas). Si une variable manque → fail-fast (postgres ou
> pydantic refusent de démarrer).
>
> Tags d'images optionnels (interpolés, avec défaut) : `FRONT_IMAGE_TAG`
> (`main-latest`), `BACKEND_IMAGE_TAG` (`latest`).
>
> ⚠️ Ne jamais committer de `.env` (gitignored).

## Tester en local

Comme sur la borne, la config vient d'un **override** (le compose de base n'a pas
ces variables). Crée un `deploy/docker-compose.override.yml` (gitignored) qui imite
ce que Fliphetic génère :

```yaml
services:
  db:
    environment:
      POSTGRES_DB: flipper
      POSTGRES_USER: flipper_user
      POSTGRES_PASSWORD: change-me
  backend:
    environment:
      DB_NAME: flipper
      DB_USER: flipper_user
      DB_PASSWORD: change-me
      LOG_LEVEL: INFO
```

```bash
cd deploy
docker compose config   # valide (override auto-chargé car même dossier, sans -f)
docker compose up        # pull + démarre tout
```

> L'auto-chargement de `docker-compose.override.yml` ne marche **que** sans `-f`
> (depuis le dossier `deploy/`). Si tu tiens à `-f`, passe les deux fichiers :
> `-f deploy/docker-compose.yml -f deploy/docker-compose.override.yml`.

Puis ouvre l'un des ports éphémères affichés (`docker compose ... ps`) — chaque
front sert son écran à la racine `/`.

## Pré-requis images

Le compose tire des images publiées par les CI de `Flipper_front`
(matrice 3 écrans, **avec le proxy nginx `/api`+`/ws`**) et de `Flipper_back`.
Si la table 3D ou l'API ne répond pas, vérifier que ces images sont à jour.
