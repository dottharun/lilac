# Lilac

Experimental interpreter written from scratch in c
- based on Thorsten Ball's ["Writing An Interpreter In Go - Monkey Lang"](https://interpreterbook.com/)

https://github.com/user-attachments/assets/82c04eb4-125a-4da2-b99a-4adb903be9ea

https://github.com/user-attachments/assets/34f7684f-48aa-4e39-8a03-aed9777b29d9

## setup
### Build Deps
- clang > 18.1.8
- curl

### Build
- `make deps` to setup include file deps
- `make` or `make all` to build the app in `out/`

### Dev Env
- uses [bear](https://github.com/rizsotto/Bear) to build compilation_database for lsp
    - `make compile-db`

### Testing
- `make test` builds the test_runner in `out/`, then just run it with `./out/test_runner`
- Lilac uses [greatest](https://github.com/silentbicycle/greatest) for the test suites
    - so separate test suites can be ran via `-s` flag with many other options

## usage
Build the binary output with `make` command.

For lexer output:
```sh
./out/lilac --lexer
```

For parser output:
```sh
./out/lilac --parser
```
