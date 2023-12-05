# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.0] - [unreleased]

This version requires SDK `5.8.0` or higher.

### Added

- Add support for Architecture version `8.1.0`.
  The _Multi Applications_ (`kf`) module has been removed and replaced by the property `com.microej.runtime.capability`.
  This property can be set either in the configuration project (in `mjvm/mjvm.properties` file) or as an MMM property to override the configuration project.
  The MMM property `com.microej.platformbuilder.module.multi.enabled` is deprecated but still supported when `com.microej.runtime.capability` is not set.
- Add execution of artifact checker for `CHANGELOG.rst` and `LICENSE.txt` (`README.rst` is ignored).
- Add publication of `NOTICE.txt` and `MICROEJ_SDK_EULA.txt` files. The notice describes the list of files licensed under SDK EULA.
- Add include/exclude pattern when copying the BSP project.
- Rename Platform to VEE Port in module.ivy.

### Fixed

- Fix execution of artifact checker on `-configuration` project instead of `-fp` project.
- Fix build and run Linux scripts end-of-line (EOL) characters if the VEE port was built on Windows.
- Fix update in some cases of The _Multi Applications_ (`kf`) module in the `.platform` file (only when `com.microej.platformbuilder.module.multi.enabled` was set to `true`).

## [2.0.0] - [2023-06-29]

### Added

- Add support to execute the optional RIP scripts that configure the RIP's module during the build of the VEE Port (`build/autoConfigurationXXX.xml` or `build/platform-XXX.ant`).

### Changed

- Separate files required for SDK version `5.x` from files required for Architecture version `7.x`.

### Fixed

- Fix new empty line generated in  the `.platform` file when `com.microej.platformbuilder.module.multi.enabled` is set to `true`. This prevents a useless change in the VCS.

## [1.4.0] - [2021-07-07]

### Changed

- Copy the files LICENSE.txt, README.rst, RELEASE_NOTES.rst and CHANGELOG.rst to the artifact published if the files are available.
- Plug the artifact checker to allow the check the artifact published (disabled by default).

## [1.3.1] - [2021-06-30]

### Changed

- Moved `module.ivy` examples of dependencies in comments with description

## [1.3.0] - [2021-05-11]

### Added

- Add MicroEJ SDK tool to deploy an executable file on device using the BSP run script
- Add an option to check if the platform project exists

### Changed

- Execute bash script file (`.sh`) with `/bin/bash`.  This ensure the build and run scripts are properly executed regardless of their file permissions.
- Use placeholders for platform name and platform version in `default.platform` to make it obvious it is generated automatically.

### Fixed

- Execute `.bat` scripts on Windows 11 instead of `.sh` scripts.

## [1.2.0] - 2021-04-16

### Added

- Add `default.platform` to allow any empty project to be converted as a Platform project (no longer need to import a MicroEJ Architecture first).
- Set default module version to `0.1.0` as other MicroEJ SDK skeletons.
- Set default module organization to `com.mycompany` as other MicroEJ SDK skeletons.
- Use a new private configuration `embedded` as the default for the platform dependencies.  This prevents an instance of "Too many loaded Platforms" error in `platform-loader`.

### Fixed

- Fix documentation in `bsp.properties`:

  - The variable `${project.prefix}` is no longer supported.
  - The variables `*.relative.dir` are relative to the BSP root directory.

- Fix execution of `run.[bat|sh]` in the directory where is defined `application.out`.
- Fix module configurations for correct Platform module import (especially in a module repository)
- Fix module build crash when no README file is declared (optional)

## [1.1.0] - 2021-01-08

### Changed

 - Added scripts and `module.ivy` file for Platform module build and publication.

## [1.0.0] - 2020-06-24

### Added

  - Initial version with scripts for BSP connection.
  
---
_Copyright 2020-2023 MicroEJ Corp. All rights reserved._
_Use of this source code is governed by a BSD-style license that can be found with this software._
