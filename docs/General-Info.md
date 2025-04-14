# General Info

This page lists general info that should be known about the project.

## Build types

There are three main types of Phobos builds:
- *stable builds* - those are numbered like your regular versions (something close to semantic versioning, e.g. version 1.2.3 for example) and ideally should contain no bugs, therefore are safe to use in mods;
- *development builds* - those are the builds which contain functionality that needs to be tested. They are numbered plainly starting from 0 and incrementing the number on each release. Mod authors still can include those versions with their mods if they want latest features, though we can't guarantee lack of bugs;
- *nightly builds* - bleeding edge versions which can include prototypes, proofs of concepts, scrapped features etc., in other words - we can't guarantee anything in those builds and they absolutely should NOT be used in mod releases and should only be used to help with development and testing.

### Disabling development build warning

**DISCLAIMER:** We understand that everyone wants to try and use the new features as soon as they're released, but we can't do all the testing ourselves, so we only test the functionality on a basic level. We ask everyone who uses the new development build first to **test the new changes in every possible way first before disabling the development build warning** and proceeding to include the build in your mod release. This would allow us to concentrate on implementing the actual features, which is the most complex task. Learn more on testing [here](Contributing.md#testing).

You can hide the warning by specifying the build number after `-b=` as a command line argument (for example, `-b=1` would hide the warning for development build #1 of Phobos).

## Saved games filtering

Phobos fully supports saving and loading thanks to prototype code from publicly released Ares 0.A source and it implements it's own filtering which shouldn't conflict with Ares save filtering. Save games between different versions are incompatible due to changes to Phobos extension classes which are present in almost every build release. The filtering mechanism, hovewer, doesn't apply to nightly versions - those use latest development build number on which this nightly is based on. While different nightly version saves may be listed, they are most likely incompatible in case there were changes to extension class fields.

## Compatibility

While Phobos is standalone, it is designed to be used alongside [Ares](https://ares.strategy-x.com) and [CnCNet5 spawner](https://github.com/CnCNet/cncnet-for-ares). Adding new features or improving existing ones is done with compatibility with those in mind.

While we would also like to support HAres we can't guarantee compatibility with it due to the separation of it's userbase and developers from international community. We welcome any help on the matter though!
