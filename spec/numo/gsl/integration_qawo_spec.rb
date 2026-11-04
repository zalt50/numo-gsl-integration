# frozen_string_literal: true

RSpec.describe Numo::GSL::Integration do
  describe ".qawo" do

    let(:integrate) do
      lambda do |&block|
        described_class.qawo(
          0.0,
          1.0,
          1.0,
          described_class::COSINE,
          &block
        )
      end
    end

    it_behaves_like("an integration callback")

    it "integrates with a cosine weight", :aggregate_failures do
      result, error = described_class.qawo(
        0.0,
        Math::PI / 2.0,
        1.0,
        described_class::COSINE
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(1.0))
      expect(error).to(be >= 0.0)
    end

    it "integrates with a sine weight", :aggregate_failures do
      result, error = described_class.qawo(
        0.0,
        Math::PI,
        1.0,
        described_class::SINE
      ) { 1.0 }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end

    it "raises GslError directly when GSL integration fails" do
      error = begin
        described_class.qawo(
          0.0,
          1.0,
          1.0,
          described_class::COSINE,
          epsabs: 0.0,
          epsrel: 0.0
        ) { 1.0 }
      rescue Numo::GSL::GslError => e
        e
      end

      expect(error).to(be_instance_of(Numo::GSL::GslError))
    end

    it "accepts integration options", :aggregate_failures do
      result, error = described_class
        .qawo(
          0.0,
          Math::PI,
          1.0,
          described_class::SINE,
          epsabs: 1e-12,
          epsrel: 1e-12,
          limit: 100
        ) { 1.0 }

      expect(result).to(be_within(1e-10).of(2.0))
      expect(error).to(be >= 0.0)
    end
  end
end
