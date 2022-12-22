# Changelog

## [0.6.0](https://github.com/PoetaKodu/pacc/compare/v0.5.0-prealpha...v0.6.0) (2022-12-22)


### Features

* **app:** prefer `pacc.json` instead of `cpackage.json` as main package file. Deprecate the `cpackage.json` ([c872896](https://github.com/PoetaKodu/pacc/commit/c872896ff5f055dc73faa663a38c3a7cd1fdcd69))
* **app:** remove unnecessary `-prealpha` suffix, bump version to `0.6` ([7ae27ae](https://github.com/PoetaKodu/pacc/commit/7ae27ae4e590a84b7221f67568b503337c265bb5))
* better error messages, hints ([2eb2057](https://github.com/PoetaKodu/pacc/commit/2eb2057cb34d9318d31e0b3555e669765050f528))
* better lua handling ([2eb2057](https://github.com/PoetaKodu/pacc/commit/2eb2057cb34d9318d31e0b3555e669765050f528))
* **ci:** testing branch ci ([a7f2cba](https://github.com/PoetaKodu/pacc/commit/a7f2cba018b5ca7fb1c49cf7346d63f562285dd3))
* git shouldn't ignore `deps/pacc.json` ([bdcf7b5](https://github.com/PoetaKodu/pacc/commit/bdcf7b585a6b0ae6285762e697d09046aa1598ba))
* more helper types ([22b636e](https://github.com/PoetaKodu/pacc/commit/22b636efba5e4252aa12e7f87434aff45c5cacb2))
* new, more flexible way to parse and handle program arguments ([2eb2057](https://github.com/PoetaKodu/pacc/commit/2eb2057cb34d9318d31e0b3555e669765050f528))
* override lua library path with `--lua-lib=path` ([2eb2057](https://github.com/PoetaKodu/pacc/commit/2eb2057cb34d9318d31e0b3555e669765050f528))
* override premake5 executable with `--premake5=command` ([2eb2057](https://github.com/PoetaKodu/pacc/commit/2eb2057cb34d9318d31e0b3555e669765050f528))
* package events and tasks ([4cd883b](https://github.com/PoetaKodu/pacc/commit/4cd883b55df77b8a80f672f1735e4dc09541bcff))


### Bug Fixes

* (workaround) use `std::hash&lt;std::string&gt;` as a working alternative to `fs::path` hash (not working on GCC-11 Ubuntu) ([af62e7c](https://github.com/PoetaKodu/pacc/commit/af62e7c60da46a115c95f7a971b1562276d461a7))
* added `-fPIC` on gmake generator to fix linking error ([af62e7c](https://github.com/PoetaKodu/pacc/commit/af62e7c60da46a115c95f7a971b1562276d461a7))
* **app:** bring back `cpackage.json` for legacy builds ([0ca8adb](https://github.com/PoetaKodu/pacc/commit/0ca8adb4694b981fc6b849343a4452e09314fc8a))
* comparing wrong iterators ([da85636](https://github.com/PoetaKodu/pacc/commit/da856360b114b01a763945ce553e1f77f71b2e00))
* detection of binaries didn't work on non-default build settings ([2eb2057](https://github.com/PoetaKodu/pacc/commit/2eb2057cb34d9318d31e0b3555e669765050f528))
* generating compile commands didn't work due to invalid path calculation ([1e5c3b0](https://github.com/PoetaKodu/pacc/commit/1e5c3b0f6c7a80c02ecdd277adcd09671db24471))
* long lasting bug that caused `pacc.json` to not work. ([af9ea71](https://github.com/PoetaKodu/pacc/commit/af9ea71413adbcf1edd66e93c1b444c8ca62715a))
* misleading comments ([4ef52ec](https://github.com/PoetaKodu/pacc/commit/4ef52ec59abbd4b39caa2791b5fd442883f0ea2f))
* missing `Filesystem` header ([8bf8b95](https://github.com/PoetaKodu/pacc/commit/8bf8b951c1ce0a6a69b02415b34d66ea7727a284))
* package loading from parent folder not being inside `pacc_packages` ([6ebbb93](https://github.com/PoetaKodu/pacc/commit/6ebbb93d66e8a44398d525162d57fbd39b581caf))
* unwanted `;` ([9d2965e](https://github.com/PoetaKodu/pacc/commit/9d2965ef2adf8b5357e49158dcc77102101cc41b))
* use junctions instead of symlinks on Windows ([c9b2448](https://github.com/PoetaKodu/pacc/commit/c9b2448ba7f31f0a1595e9943f1aaf45de787de1))

## [0.5.0-prealpha](https://github.com/PoetaKodu/pacc/compare/0.4.1-prealpha...v0.5.0-prealpha) (2022-06-26)


### Features

* **deployment:** added continuous integration ([4fbcb31](https://github.com/PoetaKodu/pacc/commit/4fbcb313c2532f4dcae65a30fa4f14ed9fdc307f))
* **deployment:** added deployment content ([4b67fdf](https://github.com/PoetaKodu/pacc/commit/4b67fdfd638764f1e46f029fe7a30debe229f53b))
* **deployment:** added the changelog file and the version file. ([5becbbe](https://github.com/PoetaKodu/pacc/commit/5becbbe17654cfc8e6f26a2491dcf5c366dbef87))
* **deployment:** follow the conventional commits and semver2 (bump version to `0.5.0-prealpha`) ([ec41256](https://github.com/PoetaKodu/pacc/commit/ec412569f53c2eb3e3db76252fde1f5a5ddb0a1b))


### Bug Fixes

* **deployment:** commands were incorrectly executed ([86ff89c](https://github.com/PoetaKodu/pacc/commit/86ff89c6b5bbc677f484cae0fbb7c93d40b7e0c5))
* **deployment:** ensure that file and directory attributes are preserved ([758add7](https://github.com/PoetaKodu/pacc/commit/758add73ce83d46d3bc715750d24b09c002567d9))
* **deployment:** generated folders are now valid ([d6cb337](https://github.com/PoetaKodu/pacc/commit/d6cb337eafaae11a526a170d2eb30ac76ada27e5))
* **deployment:** trigger on generic `*.*.*` tag pattern ([f120987](https://github.com/PoetaKodu/pacc/commit/f120987dde5e1f9e1b01dc848f8801eecbf64e04))
* **deployment:** wrong path to Linux premake5 binary on Windows build. ([4b775f5](https://github.com/PoetaKodu/pacc/commit/4b775f5a39cfcd257b539a7b61a315a1ca57bfcf))
