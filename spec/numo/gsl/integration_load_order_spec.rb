# frozen_string_literal: true
require "open3"
require "rbconfig"

RSpec.describe Numo::GSL::Integration do
  def run_fixture(name)
    fixture = File.expand_path("../../fixtures/#{name}.rb", __dir__)
    stdout, stderr, status = Open3.capture3(RbConfig.ruby, "-Ilib", fixture)
    [status, stdout + stderr]
  end

  it "loads independently and installs a GSL error handler" do
    status, output = run_fixture("integration_first")
    expect(status).to(be_success, output)
  end

  it "replaces the numo-gsl-alt handler when loaded afterward" do
    skip("numo-gsl-alt is not installed") if Gem::Specification.find_all_by_name("numo-gsl-alt").empty?
    status, output = run_fixture("numo_gsl_first")
    expect(status).to(be_success, output)
  end
end
