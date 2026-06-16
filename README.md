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

Injectées par le dashboard de la borne (ou un `.env` local — voir `.env.example`) :

| Variable | Secret | Défaut | Rôle |
|----------|--------|--------|------|
| `POSTGRES_PASSWORD` | **oui** | `flipper_password` | mot de passe Postgres |
| `POSTGRES_DB` | non | `flipper` | nom de base |
| `POSTGRES_USER` | non | `flipper_user` | utilisateur Postgres |
| `FRONT_IMAGE_TAG` | non | `main-latest` | tag des images front |
| `BACKEND_IMAGE_TAG` | non | `latest` | tag de l'image backend |
| `LOG_LEVEL` | non | `INFO` | niveau de log backend |

> ⚠️ Ne jamais committer de `.env` (gitignored). Sur la borne, `POSTGRES_PASSWORD`
> doit être fourni par le dashboard.

## Tester en local

```bash
cp .env.example .env          # ajuste POSTGRES_PASSWORD
docker compose -f deploy/docker-compose.yml config   # valide le compose
docker compose -f deploy/docker-compose.yml up        # pull + démarre tout
```

Puis ouvre l'un des ports éphémères affichés (`docker compose ... ps`) — chaque
front sert son écran à la racine `/`.

## Pré-requis images

Le compose tire des images publiées par les CI de `Flipper_front`
(matrice 3 écrans, **avec le proxy nginx `/api`+`/ws`**) et de `Flipper_back`.
Si la table 3D ou l'API ne répond pas, vérifier que ces images sont à jour.
