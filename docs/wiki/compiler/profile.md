# Profile

Profiles are located at `profile/` folder.

Profiles are collections of compilation options.

The project's base configuration is `your_project/tolza.toml`. It defines the default compilation options and the project's configuration. Profiles located in `profile/` override or extend these defaults.

Define build profile composition in the root `profile` field of `your_project/tolza.toml`. Profiles names must correspond to files in the `profile/` folder.

> It's possible to define a composition of profiles: `profile = "linux|posix|release"`

The composition affects the build file name and executable file name :

e.g. `profile = "windows"` -> `my_app-windows`, `profile = "linux|arm"` -> `my_app-linux-arm`

> The composition order affects the final configuration. Profiles are applied from left to right. When multiple profiles override the same option, the value from the last applied profile takes precedence.

> For fine-grained profile composition, profiles can define options that are disabled by default. Uncommenting or enabling these options allows them to participate in the composition. Merge modes can also be specified for selected option sets.

## Profile composition

```mermaid
flowchart TD
  AA[my_app] --> A[tolza.toml]
  subgraph PROFILES["/profile"]
    C[debug.toml]
    C3[linux.toml]
    C2[windows.toml]
    C1[release.toml]
  end
  A --> PROFILES

  A -. base options .-> D["profile = 'linux|release'"]
  D -. merge .-> C3
  C3 -. merge .-> C1
  C1 -. merge .-> C5[profiles command]
  C5 -. merge .-> C4[Command args other options]
  C4 .->|--profiles '...'| C5

  C4 .-> E[Final compilation options]
  E -. compilation .-> G["/build/linux-release/my_app-linux-release.x86_64"]
```
