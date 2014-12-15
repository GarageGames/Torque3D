# Torque 3D contribution guidelines

So you want to help Torque out by contributing to the repo? That's awesome!
We just ask that you'd give this document a quick read to get yourself familiar with the process.
Do you want to [request a feature](#request-a-feature)?
Create a [pull-request](#create-a-pull-request) to contribute your own code to the engine?
[Report an issue](#report-an-issue) you've discovered?

## Report an issue

Before you report an issue with the engine, please [search](https://github.com/GarageGames/Torque3D/issues) and quickly make sure someone else hasn't obviously reported it.
If you're not sure if it's the same issue, go ahead and comment on it!
Once you're certain you've found a new issue, hit the [big green button](https://github.com/GarageGames/Torque3D/issues/new) and please include the following information:

 * Your platform and compiler, if you're not using a precompiled binary
 * Steps to reproduce the issue, if _at all_ possible
 * If it's related to graphics, your GFX card and driver details.

## Create a pull-request

We ask that potential contributors read our [pull-request guidelines](http://torque3d.org/contribute/#pull-request-guide) before opening a PR.
We also have some [code style guidelines](https://github.com/GarageGames/Torque3D/wiki/Code-Style-Guidelines).
Here's a quick guide to the branches in this repo that you might think of targeting a PR at:

### The master branch

The repository's `master` branch is where we make releases.
It's supposed to be stable at all times - or as stable as we can make it - and only gets updated when a new version comes out.
Any pull-requests to the master branch will have to be rejected - sorry :(.

### The development branch

The `development` branch is where most development happens.
It is the target for the next 'middle' version of the engine (the 6 in 3.6.1, for example).
This means we will add new features, and refactor code if it doesn't break existing games made with the engine _too_ much*.
Most pull requests to `development` can be accepted if we like your code - unless they would potentially break users' games.

*How much is _too_ much is for the Steering Committee to decide.

### The development-3.6 branch

The `development-3.6` branch is where we will make bugfixes and small patches to the previous stable 'middle' version.
This branch is where the 'small' versions will be created - 3.6.2, 3.6.3, etcetera.
So if you have a bugfix or tiny enhancement that doesn't require anyone to change their game, it'd be best appreciated in this branch.

### TLDR

Don't make any PRs to `master`.
PR new features and large fixes/refactorings to `development`.
PR bugfixes to `development-3.6`.

## Request a feature

We ask that all feature requests be discussed in the [GarageGames forums](http://www.garagegames.com/community/forums), our [IRC channel](http://torque3d.wikidot.com/community:chat), or on our [UserVoice feature request voting page](https://garagegames.uservoice.com/forums/178972-torque-3d-mit/filters/top) before making an issue here.
If your idea is popular, we'll hear of it and probably make an issue ourselves, if we agree.

Even better - don't request a feature, start working on it!
This engine isn't going to improve itself ;).
