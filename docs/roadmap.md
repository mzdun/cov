# Roadmap

- [x] RAII-based API for `libgit2`
- [x] Directory initialization/discovery
- [x] Add file serdes
  - [x] `report`
  - [x] `build`
  - [x] `files`
  - [x] `line_coverage` _(0.18.0)_
  - [x] `function_coverage` (_0.20.0_, mzdun/cov#48)
  - [ ] `branch_coverage` (mzdun/cov#61)
- [x] Wrap I/O in Z streams _(0.2.0-alpha)_
- [x] Idea of references, tags and "branches" _(0.3.0-alpha)_
- [x] Report format for `log` _(0.4.1-alpha)_
- [x] Add diff to `git2-c++` with file movement _(0.5.0-alpha)_
- [x] Add highlighting API
  - [x] Add hilite for C++ _(0.6.0-alpha)_
- [x] Repository _(0.7.0-alpha)_
  - [x] config,
  - [x] reports and blobs,
  - [x] references,
  - [x] stats diffing,
  - ~~formatting~~,
  - ~~highlighting~~,
- [ ] Add commands _(drop alpha)_
  - [x] `cov`, builtin and `libexec/cov` _(0.8.0-beta)_
    - [x] aliases inside config
  - [x] lngs baseline _(0.9.0-beta)_
    - [x] `wmain` and spawn w/UTF8 on Windows
  - [x] `cov init` _(0.10.0-beta)_
    - [x] SetFileAttributes FILE_ATTRIBUTE_HIDDEN
  - [x] `cov config` _(0.11.0-beta)_
  - [x] flatten strings files structure _(0.11.1-beta)_
  - [x] `cov module` _(0.12.0-beta)_
    - _[module "libraries"]_\
    _name=..._\
    _dir=..._\
    _dir=..._
    - [x] rename `.covmodule` to `.covmodules` (_0.21.0_, mzdun/cov#54)
  - [x] Patch-worth:
    - [x] add win32 RC file _(0.12.1-beta)_
    - [x] merge/split .po files _(0.12.2-beta)_
  - [x] `cov report` _(0.13.0-beta)_
    - [x] update report-creation API
      - remove file builder
      - introduce report builder
      - get rid of is_dirty/is_modified
    - [x] own format (either MD5 or SHA over the content, only commit and branch)
    - [x] coveralls
    - [x] cobertura
    - [x] `cov report --amend`
    - [x] extract `make_u8path`/`get_u8path` to cov-rt
  - [x] code cleanup
    - [x] add documentation
    - [x] `cov::sth_create` -> `cov::sth::create`
    - [x] drop &beta;
  - [x] `cov log` _(0.14.0)_
  - [x] `cov tag` / `branch` / `checkout` _(0.15.0)_
  - [x] `cov reset` _(0.16.0)_
  - [x] `cov show` _(0.17.0)_
    - [x] Report / component / directory view
    - [x] File view
    - [x] Show builds and files from hash, add file info to file view (_0.21.0_, mzdun/cov#51)
  - [x] `cov report` reprise
    - [x] `cov report --prop`
    - [x] show propset in `report`
    - [x] show propset in non-`log` `show`s _(0.18.0)_
    - [x] show propset in `log` (_0.19.0_, mzdun/cov#46)
    - [x] fully alias co-existing function in report stats (_0.21.0_, mzdun/cov#55)
    - [x] `$COV_FILTER_PATH` (_0.21.1_, mzdun/cov#47)
  - [ ] `cov export` from report, build and files (mzdun/cov#58)
    - [x] schema (_0.26.1_)
    - [ ] `--json`
    - [x] `--html` (_0.26.0_)
  - [ ] `cov report` (mzdun/cov#59)
    - [ ] build set manipulation (adding, removing from HEAD)
    - [ ] `cov show` changes: take a list of properties to select only some builds
- [ ] Support for `less` (mzdun/cov#60)
- [ ] Look into tortoise and a hare (mzdun/cov#62)
- [ ] Major release
  - Update schema id and references in filters
  - [ ] Freeze translation IDs (mzdun/cov#63)
- [ ] `cov serve`
  - [ ] Boost.Beast WS
  - [ ] React frontend
  - [ ] Project list
  - [ ] Project view
    - [ ] Coverage graph
    - [ ] Latest report / report list
  - [ ] Report / component / directory view
  - [ ] File view
---
**Potential bugs:**
- refs: employ tortoise and hare in peel_target
- refs: windows fs string equivalence (possibly through canonicity when reading from FS)
---
- [x] Analytical tools _(0.15.1)_
  - [x] build with clang
  - [x] gcc
  - [x] clang?
  - [x] msvc
- [ ] Multi-platform reports
  - [ ] External build id / tag?
  - [ ] `cov report` taking more, than one report
    - with single filter?
    - with filter per input?
    - [ ] build set manipulation (adding, removing from HEAD)?
- [ ] Direct `gcov` JSON.GZ filter
- [ ] Direct `llvm-cov` filter
- [ ] Add report object cache
- [ ] Highlight partial C++ strings
- [ ] Add hilite for Python
- [ ] Add hilite for TypeScript
- [x] 💸 Cleanup initialization for `data-tz` (_0.25.0_, mzdun/cov#102)
  - [x] Properly setup `access_install` directory
  - [x] ~~Work with 7zip binary~~ Replace with `libarch`

## Next steps:

1. mzdun/cov#58 `cov export (--json | --html)` 
1. mzdun/cov#62 look into tortoise and a hare
1. mzdun/cov#59 build set manipulation (adding, removing from HEAD)
1. mzdun/cov#61 `branch_coverage`/`"bran"`
1. mzdun/cov#60 support for a pager
1. mzdun/cov#63 freeze strings
1. ??? major release
