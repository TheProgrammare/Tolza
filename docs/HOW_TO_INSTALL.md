# How to install

Go to Releases

Or build yourself:

1 - clone this repository
```bash
git clone https://github.com/TheProgrammare/Tolza.git
```

2 - install dependencies

check [dependencies](/docs/CONTRIBUTING_CODE.md#install-dependencies)

3 - install command

install toolchain: 
```
cd REPO_LOCATION/tolza/toolchain && cmake --preset release && cmake --install build/release
```

install compiler:
```
cd REPO_LOCATION/tolza/compiler && cmake --preset release && cmake --install build/release
```
