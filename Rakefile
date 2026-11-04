# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/extensiontask"
require "rspec/core/rake_task"
require "rubocop/rake_task"
require "shellwords"

Rake::ExtensionTask.new("integration") do |ext|
  ext.ext_dir = "ext/numo/gsl/integration"
  ext.lib_dir = "lib/numo/gsl/integration"
end

RSpec::Core::RakeTask.new(:spec)

desc("Compile the native extension before running specs")
task(spec: :compile)

RuboCop::RakeTask.new

RUBY_FORMAT_FILES = FileList[
  "Rakefile",
  "*.gemspec",
  "lib/**/*.rb",
  "spec/**/*.rb",
  "ext/**/*.rb"
].to_a.freeze

C_FORMAT_FILES = FileList[
  "ext/**/*.{c,h}"
].to_a.freeze

namespace(:format) do
  desc("Format Ruby, C, and Markdown files")
  task(:apply) do
    ruby_files = Shellwords.join(RUBY_FORMAT_FILES.to_a)
    c_files = Shellwords.join(C_FORMAT_FILES.to_a)

    sh("bundle exec rubyfmt -i #{ruby_files}")
    sh("clang-format --style=LLVM -i #{c_files}")
    sh("markdownlint-cli2 --fix README.md CHANGELOG.md")
  end

  desc("Check Ruby, C, and Markdown formatting")
  task(:check) do
    ruby_files = Shellwords.join(RUBY_FORMAT_FILES.to_a)
    c_files = Shellwords.join(C_FORMAT_FILES.to_a)

    sh("bundle exec rubyfmt --check #{ruby_files}")
    sh("clang-format --style=LLVM --dry-run --Werror #{c_files}")
    sh("markdownlint-cli2 README.md CHANGELOG.md")
  end
end

desc("Validate RBS signatures")
task(:rbs) do
  sh("bundle exec rbs -I sig validate")
end

desc("Lint Markdown files")
task(:markdownlint) do
  sh("markdownlint-cli2 README.md CHANGELOG.md")
end

task(default: %i[spec rubocop rbs format:check])
