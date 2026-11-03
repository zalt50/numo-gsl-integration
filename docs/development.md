# Development

After checking out the repository, install the dependencies:

```bash
bundle install
```

Build the native extension and run the complete verification suite:

```bash
bundle exec rake
```

The default Rake task performs the following checks:

- compiles the native extension;
- runs the [RSpec](https://rspec.info/) suite;
- runs [RuboCop](https://rubocop.org/);
- validates RBS signatures;
- checks Ruby files with [rubyfmt](https://github.com/fables-tales/rubyfmt);
- checks C source files with `clang-format` using the LLVM style; and
- checks `README.md` and `CHANGELOG.md` with `markdownlint-cli2`.

Check source formatting without applying changes:

```bash
bundle exec rake format:check
```

Format the Ruby and C source files:

```bash
bundle exec rake format:apply
```

RuboCop is configured with `rubocop-rake`, `rubocop-rspec`, and `rubocop-rubyfmt`.

## clangd

To enable accurate code completion, navigation, and diagnostics when editing the native extension, generate a compilation database with [Bear](https://rizsotto.github.io/Bear/):

```bash
bear -- bundle exec rake clean compile
```

The compilation database provides `clangd` with the compiler options and Ruby and GSL header paths used to build each C source file. See the [clangd documentation](https://clangd.llvm.org/design/compile-commands) for details.

The generated `compile_commands.json` contains paths specific to the local build environment and should not be committed. Regenerate it after changing the Ruby or GSL installation or other build settings.
