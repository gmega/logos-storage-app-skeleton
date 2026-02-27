# Logos Storage App Skeleton

This repository contains the basic skeleton for a CLI application for Logos Storage, using Logos Core. It serves as a companion to the [Logos Storage Module API tutorial](https://github.com/logos-co/logos-storage-module/blob/main/docs/storage-module-api.md).

## Building

This project requires [nix](https://nixos.org/download/). To build, run:

```bash
nix build
```

The application will then be available at `./result/bin/storage-app`.

## Implementing Your App

The skeleton (`main.app`) provides an entry point under `app_main`, which supplies the `LogosModules` object - referred to as `m_logos` object in the API tutorial - and allow API invocations to be done as in the tutorial. It also provides Qt-friendly synchronization primitives to block and await for operations to complete. See the [skeneton's source](./app/main.cpp) for more details.