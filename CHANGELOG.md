# Changelog

All notable changes to this project will be documented in this file. See [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) and [COMMITS.md](COMMITS.md) for commit guidelines.

## [0.12.3-beta](https://github.com/mzdun/cov/compare/v0.12.2-beta...v0.12.3-beta) (2022-07-31)

### Build System

- automatically remove docker containers ([47bdd3a](https://github.com/mzdun/cov/commit/47bdd3af3c796a7a1698e091656207dd95e32e8a))
- force unix line endings in split files ([72fe514](https://github.com/mzdun/cov/commit/72fe5143a52c7694d869e89cfb118e9ca2e32210))
- remove first version of helpers ([595de5c](https://github.com/mzdun/cov/commit/595de5ca6474d569fc88dde7ee62a3b2611514e3))
- home-brewed make scripts for new language structure ([b75104b](https://github.com/mzdun/cov/commit/b75104bc662a110a8ad6639a75ea7f53e4bc1a6f))
- **lang**: switch to combined POT/PO files ([7ae0d19](https://github.com/mzdun/cov/commit/7ae0d1987f60eb85c455a44edcad7a38ae7e537a))
- **lang**: remove per-lngs gettext files ([4f9be6e](https://github.com/mzdun/cov/commit/4f9be6e1486768d113af826b54156a0799dd6a18))

### Code Style

- **spell-check**: add polib names to dictionary ([603555b](https://github.com/mzdun/cov/commit/603555bf5c06919a95e47345bea4c2dd861f2b10))

## [0.12.2-beta](https://github.com/mzdun/cov/compare/v0.12.1-beta...v0.12.2-beta) (2022-07-31)

### Bug Fixes

- update the 0.12.1 to flat layout ([79883ac](https://github.com/mzdun/cov/commit/79883aceb4e86905b66a832d7e457df2c2727d1b))
- `docs` should be fully treated as a `fix` ([14ace6a](https://github.com/mzdun/cov/commit/14ace6ac439ce90c60a516198fa60eb804902ec2))
- update commit footer description ([803400d](https://github.com/mzdun/cov/commit/803400d4a15b6dd9a4f3ca0e847ac7dfb82069d2))
- hide non-fix/non-feat from changelog ([4134039](https://github.com/mzdun/cov/commit/413403906c1e1118c8f1be5c960f43b10ea8baac))

## [0.12.1-beta](https://github.com/mzdun/cov/compare/v0.12.0-beta...v0.12.1-beta) (2022-07-30)

### Bug Fixes

- **lang**: add win32 RC description string ([85f938a](https://github.com/mzdun/cov/commit/85f938ac03064d04cef7cbcb54599a1f54894843))
- update roadmap, add conventional commits ([dbfe43b](https://github.com/mzdun/cov/commit/dbfe43b528069d7059d5173e17d8da73f1c89bb2))

## [0.12.0-beta](https://github.com/mzdun/cov/compare/v0.11.1-beta...v0.12.0-beta) (2022-07-29)

### New Features

#### Application Library

- add cov module code ([872fdcb](https://github.com/mzdun/cov/commit/872fdcbe0451d3e671cc64fba765395bde496164))

#### API

- add modules to cov-api ([a80d769](https://github.com/mzdun/cov/commit/a80d769b8e1ca6b5684827e0581a6141d7190cd8))

#### Language Strings

- add strings for cov module ([c47e875](https://github.com/mzdun/cov/commit/c47e875fc6714ef76c0fa5c7f991534a6b407778))

## [0.11.1-beta](https://github.com/mzdun/cov/compare/v0.11.0-beta...v0.11.1-beta) (2022-07-23)

### Bug Fixes

#### Language Strings

- fix string issues in cov config ([de84f34](https://github.com/mzdun/cov/commit/de84f3461368f6f45e1d172d089c0383bfb409eb))

## [0.11.0-beta](https://github.com/mzdun/cov/compare/v0.10.0-beta...v0.11.0-beta) (2022-07-19)

### New Features

#### Application Library

- add cov config code ([b7cbe23](https://github.com/mzdun/cov/commit/b7cbe23e7be46bb313565d54044335a684fa7499))

#### Language Strings

- add strings for cov config ([bce1915](https://github.com/mzdun/cov/commit/bce1915d163b05f9120bf9e59e337eae2bcf214f))

## [0.10.0-beta](https://github.com/mzdun/cov/compare/v0.9.0-beta...v0.10.0-beta) (2022-07-19)

### New Features

#### Application

- add code for cov init ([a445cae](https://github.com/mzdun/cov/commit/a445caefd24e5c26bc4a773d5469cbb9ca549303))

#### Language Strings

- adding strings for cov init ([e0981e2](https://github.com/mzdun/cov/commit/e0981e2e93356f1c88de075baf8ec6a6601c2137))

### Bug Fixes

#### Build System

- fix runner output patching on paths w/short names ([d80695a](https://github.com/mzdun/cov/commit/d80695a79b3f59cb3c353021e01a45877f8b7c71))
- fix install/pack issue with moved dirs.hh.in ([97aedc1](https://github.com/mzdun/cov/commit/97aedc1bd3089ee92ed7e8a5b8d498ed9af5c394))

## [0.9.0-beta](https://github.com/mzdun/cov/compare/v0.8.0-beta...v0.9.0-beta) (2022-07-17)

### New Features

#### Language Strings

- add strings for cov itself ([b88a155](https://github.com/mzdun/cov/commit/b88a155e3c5c2f21156d18de7e10d8d94df1e1f5))
- add strings for args::base_translator ([d5b8335](https://github.com/mzdun/cov/commit/d5b8335f40d8fb3ceac09c09834ecfb756f6801e))

#### Application

- support UTF-8/UCS-2 conversion ([f12443c](https://github.com/mzdun/cov/commit/f12443c2861170ca70d48424edf05b627ec4c8ed))

### Bug Fixes

#### App Runtime

- fix error in parser::noent ([c32cd90](https://github.com/mzdun/cov/commit/c32cd90ad1929c1c627a28b4a1e0b86f10cad215))

## [0.8.0-beta](https://github.com/mzdun/cov/compare/v0.7.2-alpha...v0.8.0-beta) (2022-07-30)

### New Features

- add tool runner (alias, builtin, external) ([b87984f](https://github.com/mzdun/cov/commit/b87984faa4a7fd15bcc57e167091f6180b6515d8))

## [0.7.2-alpha](https://github.com/mzdun/cov/compare/v0.6.0-alpha...v0.7.2-alpha) (2022-07-30)

### New Features

#### API

- add author/committer/message access to git::commit ([177b110](https://github.com/mzdun/cov/commit/177b1107a09a3b3f506d01882ee2152435e8d0c2))
- add file stats diffs ([c2bbafc](https://github.com/mzdun/cov/commit/c2bbafca28d21f508d3781ba300083cc455bc542))
- add reports, objects and blobs to repository ([7d0df3d](https://github.com/mzdun/cov/commit/7d0df3dcc2a296ef6a877bd734857e8bb0f0636a))
- embrace std::error_code more ([4323855](https://github.com/mzdun/cov/commit/43238556367a6366ec39c326d8a683915e092672))
- add DWIM and refs to repository ([5f3c9a3](https://github.com/mzdun/cov/commit/5f3c9a3997d005e8023dd3e2f15516d4cfe0c872))
- enhance config file loading ([e2b64d0](https://github.com/mzdun/cov/commit/e2b64d0e1182a5da462476ed032ffe4f21d846bd))

## [0.6.0-alpha](https://github.com/mzdun/cov/compare/v0.5.0-alpha...v0.6.0-alpha) (2022-07-08)

### New Features

#### Lighter

- add newer features to C++ ([79ec6bc](https://github.com/mzdun/cov/commit/79ec6bcc603ab36e2d2dedb69bffa665e17a9c53))
- add hilite library (with C++ highlighter) ([9237b59](https://github.com/mzdun/cov/commit/9237b5944664b7041c58b9ee38302f3775048874))

## [0.5.0-alpha](https://github.com/mzdun/cov/compare/v0.4.1-alpha...v0.5.0-alpha) (2022-07-05)

### New Features

#### API

- find a way to get list of renamed files ([61152fc](https://github.com/mzdun/cov/commit/61152fc6ec5db10fbd19daf839c8171c8f58bb15))

## [0.4.1-alpha](https://github.com/mzdun/cov/compare/v0.4.0-alpha...v0.4.1-alpha) (2022-07-04)

### Other Changes

- adjust conan cache recovery ([0b267d2](https://github.com/mzdun/cov/commit/0b267d20bc4895d1edcd477f00cb2ef1f5f46259))
- print current conanfile hash ([4720c8f](https://github.com/mzdun/cov/commit/4720c8f85abe899590148a365ba4ea704c870076))

## [0.4.0-alpha](https://github.com/mzdun/cov/compare/v0.3.0-alpha...v0.4.0-alpha) (2022-07-04)

### New Features

#### API

- support %w(...) ([1748098](https://github.com/mzdun/cov/commit/174809847abd48fe762fb2a0ca7de96539461822))
- support %C(...) ([113a4e0](https://github.com/mzdun/cov/commit/113a4e01b75a39e7d8204234b41d922426f64a17))
- add subset of pretty-formats ([22d85ed](https://github.com/mzdun/cov/commit/22d85edbdaa428581c6d90a29870604d1d614671))

## [0.3.0-alpha](https://github.com/mzdun/cov/compare/v0.2.0..v0.3.0-alpha) (2022-07-01)

### New Features

#### API

- add tags and "branches" ([36109a1](https://github.com/mzdun/cov/commit/36109a1c35e0d5cf3e5e68d896c8b1b4be565525))
- add reference API ([43d8fc6](https://github.com/mzdun/cov/commit/43d8fc666fc587f9c80c33b249f6f69e6143f1d7))
