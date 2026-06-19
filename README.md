# flipper-deploy

Déploiement de la stack **Flipper Virtuel** sur une borne **Fliphetic**.

Ce repo contient le manifeste Fliphetic + un Docker Compose qui tire des
**images déjà publiées** (aucun build serveur des apps), **et** le firmware
ESP32 (PlatformIO) flashé sur la puce au Load. Le code applicatif vit dans
`Flipper_front` / `Flipper_back`.

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
- **Identifiant borne** (`BORNE_ID`) → **littéral** dans le compose, **pas** au
  dashboard. Il doit correspondre au `VITE_BORNE_ID` gravé **au build** des images
  front (défaut `flipper-cabinet-1`) : c'est le canal WebSocket partagé par les 3
  écrans. Au dashboard, un mismatch empêcherait les écrans de se rejoindre. Pour
  changer d'id : rebuild des fronts avec `VITE_BORNE_ID` + même valeur dans le compose.

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

## Firmware ESP32 (boutons physiques)

À la racine : `platformio.ini` + `src/` + `lib/` + `include/` (copie de
`Flipper_firmware`). Au **Load**, Fliphetic flashe l'ESP32 avec ce firmware : il
lit les boutons (GPIO) et publie chaque appui sur MQTT
(`pinball/<device>/input/button`), consommé par le backend.

⚠️ L'ESP32 est **partagé** entre groupes : chaque Load reflashe la puce. Si un
autre groupe a loadé depuis, **reload ce projet** pour récupérer ton firmware
(sinon l'ESP32 publie le format d'un autre groupe et tes boutons ne remontent pas).

Variables (dashboard) lues au build du firmware via `build_flags` :
`WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_BROKER_IP` (IP réseau de la borne),
`MQTT_BROKER_PORT`, `DEVICE_ID`.

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
