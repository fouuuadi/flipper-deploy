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

Le dashboard Fliphetic **injecte les variables dans les conteneurs** (au runtime),
pas dans le process `docker compose` — l'interpolation `${...}` du compose ne les
verrait donc pas. Conséquence :

- **Non-secrets** (`POSTGRES_DB`, `POSTGRES_USER`, `DB_*`, topologie réseau,
  `LOG_LEVEL`, ports…) → **valeurs littérales** dans `deploy/docker-compose.yml`.
- **Secret** (le mot de passe) → **injecté par le dashboard**, non déclaré dans le
  compose. Le backend lit `DB_PASSWORD` et l'image postgres lit `POSTGRES_PASSWORD`,
  donc il faut le fournir sous **les deux noms** (même valeur) :

| Variable dashboard | Conteneur | Rôle |
|--------------------|-----------|------|
| `POSTGRES_PASSWORD` | `db` | mot de passe superuser Postgres |
| `DB_PASSWORD` | `backend` | mot de passe de connexion DB du backend |

Si l'une manque → fail-fast (postgres refuse de s'initialiser / le backend lève une
`ValidationError`).

> Tags d'images optionnels (interpolés, avec défaut) : `FRONT_IMAGE_TAG`
> (`main-latest`), `BACKEND_IMAGE_TAG` (`latest`).
>
> ⚠️ Ne jamais committer de `.env` (gitignored).

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
