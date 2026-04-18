# TMUF Chat Emoji Mod — reverse notes + prototype

Ce dossier sert de base de travail pour le mod TMUF qui intercepte les messages reçus, les lit localement, puis remplace des tokens du style `:wave:` par des codes du pack de font (`$fff回`, etc.) afin de modifier **uniquement l'affichage local**.

## État actuel confirmé

- Hook stable avec **MinHook** sur `CGameNetwork::OnChatReceived`.
- Le buffer d'historique chat est à l'offset `0x88` dans `CGameNetwork`.
- Layout confirmé du buffer :
  - `int count`
  - `void* data`
  - `int capacity`
- Les lignes de chat observées dans `field_0x88` se comportent comme des enregistrements de 8 octets :
  - `int len`
  - `wchar_t* text`
- Le message le plus récent est à l'index `0`, cohérent avec `InsertNewElemAt(..., 0)`.
- La limite locale d'historique est d'environ **40 lignes** (logique liée à `field_0xB0`).
- La modification **en place** du `wchar_t* text` après le retour de `OnChatReceived` fonctionne localement.
- Les remplacements fonctionnent aussi **au milieu d'une phrase**.
- Le matching doit être **insensible à la casse**.

## Fichiers

- `dllmain.cpp` : prototype de DLL avec hook, lecture du chat et remplacement local d'emotes.
- `REVERSE_NOTES.md` : résumé du reverse utile.
- `EMOTES_MAP.md` : mapping token -> code font pack.
- `TODO.md` : prochaines étapes.
- `.gitignore` : ignore classique VS/MSBuild.

## Dépendance

Le prototype suppose **MinHook** intégré au projet (vendored dans le repo ou copié dans le projet Visual Studio).

## But court terme

- Stabiliser les remplacements répétés.
- Étendre la table d'emotes.
- Gérer proprement les remplacements qui **agrandissent** la chaîne.
