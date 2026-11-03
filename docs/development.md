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

The native extension requires compiler options for the Ruby and GSL headers. To make these options available to `clangd`, generate a compilation database with [Bear](https://github.com/rizsotto/Bear).

First, build the native extension:

```bash
bundle exec rake compile
```

Then rebuild the extension with Bear and create a symbolic link to the compilation database in the repository root:

```bash
build_dir="tmp/$(ruby -rrbconfig -e 'print RbConfig::CONFIG["arch"]')/integration/$(ruby -e 'print RUBY_VERSION')"
make -C "$build_dir" clean
bear --output "$build_dir/compile_commands.json" -- make -C "$build_dir"
ln -sf "$build_dir/compile_commands.json" compile_commands.json
```

The compilation database contains the compiler options used to build the native extension, including the Ruby and GSL header paths. It is specific to the local build environment and should not be committed to the repository. Regenerate it after changing the Ruby or GSL installation or other build settings.
