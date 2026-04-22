# TODO

## Short term

- [ ] Stabilize replacement on all messages without false duplicates.
- [ ] Centralize the emote table.
- [ ] Make matching fully case-insensitive.
- [ ] Log only the real line 0 cleanly.

## Medium term

- [ ] Handle replacements that increase the string length.
- [ ] Distinguish system messages from player messages if needed.
- [ ] Test whether formatted-cache invalidation is needed for visual refresh.
- [ ] Add a small debug mode that can be enabled or disabled.

## Long term

- [ ] Create a proper clean module instead of keeping everything centered around `dllmain.cpp`.
- [ ] Add external configuration for the emote map.
- [ ] Plan support for multiple font packs.
- [ ] Document offsets and functions for each build.
