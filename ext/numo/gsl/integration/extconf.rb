# frozen_string_literal: true

require "mkmf"
require "rubygems/version"

gsl_config = find_executable("gsl-config")
abort("gsl-config is required") unless gsl_config

minimum_gsl_version = Gem::Version.new("2.8")
gsl_version = Gem::Version.new(`#{gsl_config} --version`.strip)
abort("GSL 2.8 or later is required (found #{gsl_version})") if gsl_version < minimum_gsl_version

$CFLAGS << " #{`#{gsl_config} --cflags`.strip}"
$libs = "#{`#{gsl_config} --libs`.strip} #{$libs}"

header = "gsl/gsl_integration.h"
abort("#{header} is required") unless have_header(header)
abort("GSL Lebedev integration support is required") unless have_func("gsl_integration_lebedev_alloc", header)

create_makefile("numo/gsl/integration/integration")
