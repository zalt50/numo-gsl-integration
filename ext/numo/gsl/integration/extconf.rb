# frozen_string_literal: true

require "mkmf"

gsl_config = find_executable("gsl-config")
abort("gsl-config is required") unless gsl_config

$CFLAGS << " #{`#{gsl_config} --cflags`.strip}"
$libs = "#{`#{gsl_config} --libs`.strip} #{$libs}"

abort("gsl/gsl_integration.h is required") unless have_header("gsl/gsl_integration.h")

create_makefile("numo/gsl/integration/integration")
