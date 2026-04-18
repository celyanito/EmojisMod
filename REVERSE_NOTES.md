# Reverse notes — TMUF chat

## Fonction clé

- `CGameNetwork::OnChatReceived`
  - hook stable avec MinHook sur le build actuel.
  - point d'entrée idéal pour capter les messages reçus avant affichage local final.

## Buffer chat confirmé

Dans `CGameNetwork` :

- `field_0x88` = historique des textes de chat.
- Layout observé du buffer :

```cpp
struct RawBuf {
    int count;
    void* data;
    int capacity;
};
```

## Layout probable des lignes

Le `data` du buffer pointe sur un tableau d'entrées de 8 octets observées comme :

```cpp
struct RawChatLine {
    int len;
    wchar_t* text;
};
```

Comportement observé :
- index `0` = message le plus récent.
- quand un nouveau message arrive, l'historique est inséré en tête.

## Limite d'historique

Après insertion, le jeu ramène la taille des buffers `0x88 / 0x94 / 0xA0` à une limite lue autour de `field_0xB0`.
En pratique observée : **~40 lignes max**.

## Ce qui a marché

- Hook de `OnChatReceived`.
- Lecture du buffer chat après appel à l'original.
- Lecture des lignes en wide (`wchar_t*`).
- Modification locale en place du texte reçu.
- Remplacement de tokens style `:wave:` même au milieu d'un message.

## Ce qui a posé problème avant

- Hook manuel inline 5 bytes : instable à cause du prologue/SEH/security cookie.
- Tentative d'utiliser `GetUtf8` dans cette voie : crashs sur ce build.
- Anti-duplication basé seulement sur `count` et `data` : faux positif quand le buffer se stabilise.

## Direction recommandée

- Rester sur `OnChatReceived`.
- Modifier `wchar_t* text` en place après retour de l'original.
- Matching insensible à la casse.
- Pour les remplacements plus longs que le token, prévoir une vraie stratégie de réallocation ou de buffer secondaire.
