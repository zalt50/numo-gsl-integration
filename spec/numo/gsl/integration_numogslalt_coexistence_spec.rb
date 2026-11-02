# frozen_string_literal: true

require "open3"
require "rbconfig"

RSpec.describe Numo::GSL::Integration do
  def run_fixture(name)
    fixture = File.expand_path("../../fixtures/#{name}.rb", __dir__)
    stdout, stderr, status = Open3.capture3(RbConfig.ruby, "-Ilib", fixture)

    [status, stdout + stderr]
  end

  it "loads numo-gsl-alt before the integration extension" do
    status, output = run_fixture("integration_first")

    expect(status).to(be_success, output)
  end

  it "coexists when numo-gsl-alt is loaded first" do
    status, output = run_fixture("numo_gsl_first")

    expect(status).to(be_success, output)
  end
end
