# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/extensiontask"
require "rspec/core/rake_task"
require "rubocop/rake_task"

Rake::ExtensionTask.new("integration") do |ext|
  ext.ext_dir = "ext/numo/gsl/integration"
  ext.lib_dir = "lib/numo/gsl/integration"
end

RSpec::Core::RakeTask.new(:spec)

desc "Compile the native extension before running specs"
task(spec: :compile)

RuboCop::RakeTask.new

task(default: %i[spec rubocop])
