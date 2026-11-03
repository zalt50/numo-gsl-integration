# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  it "has a version number" do
    expect(Numo::GSL::Integration::VERSION).not_to(be_nil)
  end

  describe Numo::GSL::Integration::Error do
    it "inherits from Numo::GSL::GslError" do
      expect(described_class).to(be < Numo::GSL::GslError)
    end
  end
end
