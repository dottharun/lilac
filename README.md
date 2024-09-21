# Lilac

Experimental interpreter written from scratch in c
- based on Thorsten Ball's ["Writing An Interpreter In Go - Monkey Lang"](https://interpreterbook.com/)

# Build Deps
- clang > 18.1.8
- wget

# Build
- `make deps` to setup include file deps
- `make` or `make all` to build the app in `out/`

# Dev Env
- uses [bear](https://github.com/rizsotto/Bear) to build compilation_database for lsp
    - `make compile-db`

# Testing
- `make test` builds the test_runner in `out/`, then just run it with `./out/test_runner`
- Lilac uses [greatest](https://github.com/silentbicycle/greatest) for the test suites
    - so separate test suites can be ran via `-s` flag with many other options
