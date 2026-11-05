# frozen_string_literal: true
RSpec.describe Numo::GSL::Integration do
  it "has a version number" do
    expect(described_class::VERSION).not_to(be_nil)
  end

  describe Numo::GSL::GslError do
    it "inherits from StandardError" do
      expect(described_class).to(be < StandardError)
    end
  end
end
