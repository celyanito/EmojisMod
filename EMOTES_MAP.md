# Emotes map (pack font)

Format :

- token chat : `:name:`
- remplacement local : code du font pack du style `$fffX`

## Exemples utiles déjà testés / proches du test

- `:wave:` -> `$fff回`
- `:fire:` -> `$fff们`
- `:HYPERS:` -> `$fff我`
- `:Kappa:` -> `$fff目`
- `:KEKW:` -> `$fff战`
- `:OMEGALUL:` -> `$fff佳`
- `:Prayge:` -> `$fff源`
- `:COPIUM:` -> `$fff挑`
- `:Clap:` -> `$fff系`
- `:catRose:` -> `$fff联`
- `:disbelief:` -> `$fff火`
- `:WAJAJA:` -> `$fff建`
- `:thonk:` -> `$fff测`
- `:angry:` -> `$fff连`
- `:pikawow:` -> `$fff车`
- `:goggers:` -> `$fff竞`
- `:sniffa:` -> `$fff飙`
- `:cowJAM:` -> `$fff录`
- `:HUH:` -> `$fff需`
- `:bonk:` -> `$fff主`
- `:skull:` -> `$fff问`
- `:heart:` -> `$fff被`
- `:popcorn:` -> `$fff和`
- `:turtle:` -> `$fff您`
- `:pray:` -> `$fff试`
- `:brain:` -> `$fff可`
- `:nerd:` -> `$fff能`

## Remarque importante

Certains tokens sont plus courts que le remplacement complet `$fffX`.

Exemple :
- `:o7:` est très court
- remplacement souhaité : `$fff迹`

Dans ces cas-là, un remplacement **in-place shrink-only** ne suffit plus forcément.
Il faudra soit :
- une vraie réallocation,
- soit une autre stratégie de composition/formatage.
