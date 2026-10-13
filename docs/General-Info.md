# General Info

This page lists general info that should be known about the project.

## Build types

There are three main types of Phobos builds:
- *stable builds* - those are numbered like your regular versions (something close to semantic versioning, e.g. version 1.2.3 for example) and ideally should contain no bugs, therefore are safe to use in mods;
- *pre-release builds* - previously known as *development builds*, these builds mark the start of a new release branch and are used for testing new features before they are finalized. They are numbered with a version and a pre-release suffix (e.g., 0.5-beta1, 0.5-rc2). Mod authors can include these versions with their mods to access the latest features, but we cannot guarantee the absence of bugs;
- *nightly builds* - bleeding edge versions which can include prototypes, proofs of concepts, scrapped features etc., in other words - we can't guarantee anything in those builds and they absolutely should NOT be used in mod releases and should only be used to help with development and testing.

Besides the published builds above, any build you make locally (from Visual Studio, VS Code or the build scripts) is marked as a *local build*: like nightly, it is stamped with the git commit and ref it was built from (e.g. `v0.5.0.0 @ abc1234-dirty @ refs/heads/develop`) and shows a hideable "please test" warning.

```{hint}
You can find the downloads for these versions on the document's [main page](index.md#downloads).
```

### Disabling pre-release build warning

**DISCLAIMER:** We understand that everyone wants to try and use the new features as soon as they're released, but we can't do all the testing ourselves, so we only test the functionality on a basic level. We ask everyone who uses the new pre-release build first to **test the new changes in every possible way first before disabling the pre-release build warning** and proceeding to include the build in your mod release. This would allow us to concentrate on implementing the actual features, which is the most complex task. Learn more on testing [here](Contributing.md#testing).

You can hide the warning by specifying the exact version of the build you use after `-HideVersionWarning=` as a command line argument (for example, `-HideVersionWarning=0.5.0.0-beta1` would hide the warning for the `v0.5-beta1` pre-release of Phobos). The version is shown in the warning itself and in the title of the respective release; a build only accepts its own version, so the switch has to be updated whenever you update Phobos. Nightly builds don't support hiding the warning at all.

## Saved games filtering

Phobos fully supports saving and loading thanks to prototype code from publicly released Ares 0.A source and it implements its own filtering which shouldn't conflict with Ares save filtering. Save games between different versions are incompatible due to changes to Phobos extension classes which are present in almost every build release. Pre-releases of a version share the savegame ID of the stable release they lead up to, so their saves are filtered as a single version. Nightly and local builds are bleeding-edge snapshots that skip this version-based filtering entirely - each one is a unique snapshot, so their saves are never filtered by version.

## Compatibility

Phobos requires [SyringeEx](https://github.com/Phobos-developers/SyringeEx) (v0.1.0.2 or newer) as the launcher - it relies on capabilities (signalled via SyringeEx feature flags) that older Syringe versions do not provide, and will show an error and exit on startup without them.

While Phobos is standalone, it is designed to be used alongside [Ares](https://ares.strategy-x.com) and [CnCNet5 spawner](https://github.com/CnCNet/cncnet-for-ares). Adding new features or improving existing ones is done with compatibility with those in mind.

While we would also like to support HAres we can't guarantee compatibility with it due to the separation of it's userbase and developers from international community. We welcome any help on the matter though!

### API versioning

See [Interoperability](Interoperability.md#api-version-tracking).
