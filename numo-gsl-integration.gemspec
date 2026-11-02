# frozen_string_literal: true

require_relative "lib/numo/gsl/integration/version"

Gem::Specification.new do |spec|
  spec.name = "numo-gsl-integration"
  spec.version = Numo::GSL::Integration::VERSION
  spec.authors = ["Takahiro SATOH"]
  spec.email = ["zalt50cc@gmail.com"]

  spec.summary = "GSL numerical integration for Numo"
  spec.description = "Provides GNU Scientific Library numerical integration routines under Numo::GSL::Integration."
  spec.required_ruby_version = ">= 3.2.0"
  spec.license = "GPL-3.0-or-later"
  spec.homepage = "https://github.com/zalt50/numo-gsl-integration"

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://github.com/zalt50/numo-gsl-integration"
  spec.metadata["changelog_uri"] = "https://github.com/zalt50/numo-gsl-integration/blob/master/CHANGELOG.md"
  spec.metadata["rubygems_mfa_required"] = "true"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  gemspec = File.basename(__FILE__)
  spec.files = IO.popen(%w[git ls-files -z], chdir: __dir__, err: IO::NULL) do |ls|
    ls.readlines("\x0", chomp: true).reject do |f|
      (f == gemspec) ||
        f.start_with?(*%w[bin/ Gemfile .gitignore .rspec spec/ .github/ .rubocop.yml])
    end
  end
  spec.bindir = "exe"
  spec.executables = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/numo/gsl/integration/extconf.rb"]

  spec.add_dependency("numo-gsl-alt", ">= 0.1.3")
end
