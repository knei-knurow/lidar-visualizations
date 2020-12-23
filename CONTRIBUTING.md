# Contributing

When contributing to this repository, please first discuss the change you wish to make via
issue, email, or any other method with the owners of this repository before making a change.

## Git stuff

- `main` branch is stable. Releases are build from it.
- Target `dev` branch by default, unless you have a very good reason not to.

## Code stuff

We use [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) to format the code according
to the [Chromium style guide](https://chromium.googlesource.com/chromium/src/+/HEAD/styleguide/c++/c++.md).
If you want to get your PR merged, format your code with `clang-format`.

## Markdown stuff

- We format Markdown files with [Prettier](https://prettier.io/), so please, use it after you
  edited some `.md` file.
- Don't let your lines exceed 100 characters of length (it's hard to read files with lines
  longer than ~100 characters in the terminal)
